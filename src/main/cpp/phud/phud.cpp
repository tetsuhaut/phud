#include "db/Database.hpp" // std::span
#include "entities/Site.hpp"
#include "gui/Gui.hpp"
#include "gui/HistoryService.hpp"
#include "gui/TableService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "language/limits.hpp" // toSizeT
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "constants/ProgramInfos.hpp"
#include "phud/ProgramConfiguration.hpp"
#include "phud/ProgramArguments.hpp"  // ProgramArgumentsException, UserAskedForHelpException

#if defined(_WIN32)
#  include <windows.h> // WinMain. must be included before tlhelp32.h
#endif  // _WIN32

#include <csignal> // std::signal(), SIG_DFL, SIGABRT
#include <iostream>
#include <print>
#include <sstream> // std::ostringstream

// TODO: aligner les champs des classes en mémoire (padding)
// TODO: inclure le siteName dans les id
// TODO: tester toutes les sauvegardes en base
// TODO: utiliser std::source_location pour contextualiser les messages d'exception
//       cf. https://stackoverflow.com/questions/597078/file-line-and-function-usage-in-c
// GOOD PRACTISE: apply https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
// GOOD PRACTISE: make internal functions static
// GOOD PRACTISE: use pimpl for big classes
// GOOD PRACTISE: use assert, static_assert
// GOOD PRACTISE: ensure the required headers are included without relying on a specific STL implementation
// GOOD PRACTISE: pour tous les constructeurs, vérifier que tous les attributs sont initialisés
//                dans le constructeur ou dans la déclaration de classe.
// GOOD PRACTISE: use [[nodiscard]] for all pure functions, or where return value must be examined
// GOOD PRACTISE: no const class attribute
// explicit for constructors callable with one parameter
// FUTURE : try unity build
// FUTURE add code documentation (Doxygen, compatible with Javadoc format)
// BUG : crash à la sélection de la table : impossible de trouver l'historique pour table "Wichita 05 / 0,01-0,02 NL Holdem / Argent fictif"
// BUG : au 2eme choix d'historique
// TODO : détecteur d'historique pour que l'utilisateur sache quoi choisir
// TODO : liste des historiques surveillés

static Logger LOG { CURRENT_FILE_NAME };

static void logErrorAndAbort(int signum) {
  std::signal(signum, SIG_DFL);
  std::ostringstream oss;
  oss << boost::stacktrace::stacktrace();
  std::print(stderr, "{}\n", oss.str());
  LOG.critical(oss.str());
  std::raise(SIGABRT);
}

struct [[nodiscard]] LoggingConfig final {
  explicit LoggingConfig(std::string_view pattern) { Logger::setupFileInfoLogging(pattern); }
  LoggingConfig(const LoggingConfig&) = delete;
  LoggingConfig(LoggingConfig&&) = delete;
  LoggingConfig& operator=(const LoggingConfig&) = delete;
  LoggingConfig& operator=(LoggingConfig&&) = delete;
  ~LoggingConfig() { Logger::shutdownLogging(); }
};

#if defined(_WIN32)

class [[nodiscard]] ConditionalConsole final {
private:
  bool m_hasConsole { false };

public:
  ConditionalConsole() : m_hasConsole { TRUE == AttachConsole(ATTACH_PARENT_PROCESS) } {
    // if the app was launched in a console, do not create a new one
    if (m_hasConsole) {
      FILE* fp { nullptr };
      freopen_s(&fp, "CONOUT$", "w", stdout);
      freopen_s(&fp, "CONOUT$", "w", stderr);
      freopen_s(&fp, "CONIN$", "r", stdin);
      std::ios::sync_with_stdio(true);
      std::cout.clear();
      std::cerr.clear();
      std::cin.clear();
      std::print("\n");
    }
  }

  ~ConditionalConsole() {
    if (m_hasConsole) {
      std::print("\n");
      FreeConsole();
    }
  }
}; // class ConditionalConsole

/*
 * Under Windows, graphical user interfaces use 'WinMain' as a default entry point.
 * Note : main or WinMain functions can't be static or [[nodiscard]]
 */
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT) {
  const auto& argc { __argc };
  const auto& argv { __argv };
  ConditionalConsole attachToParentConsoleIfNeeded;
#  else
  int main(int argc, const char* const* const argv) {
#endif  // _WIN32
  std::setlocale(LC_ALL, "en_US.utf8");
  std::signal(SIGSEGV, logErrorAndAbort);
  std::signal(SIGABRT, logErrorAndAbort);
  std::signal(SIGINT, logErrorAndAbort);
  auto nbErr { 0 };

  try {
    const auto args { std::span(argv, limits::toSizeT(argc)) };
    const auto& [oHistoDir, loggingLevel, loggingPattern] { ProgramConfiguration::readConfiguration(args) };
    LoggingConfig _ { loggingPattern };
    Logger::setLoggingLevel(loggingLevel);
    LOG.info<"{} is starting">(ProgramInfos::APP_SHORT_NAME);
    Database db(ProgramInfos::DATABASE_NAME);
    TableService ts(db);
    HistoryService hs(db);

    if (oHistoDir.has_value()) {
      if (const auto& historyDir { oHistoDir.value() }; PokerSiteHistory::isValidHistory(historyDir)) {
        const auto& site { PokerSiteHistory::load(historyDir) };
        db.save(*site);
      }
      else {
        const auto& strDir { oHistoDir.value().string() };
        throw PhudException(fmt::format("The provided hand history directory '{}' is invalid", strDir));
      }
      LOG.info<"phud configuration:\n  loggingLevel={}\n  loggingPattern={}\n  historyDirectory={}">(toString(loggingLevel), loggingPattern, oHistoDir.value().string());
    }

    Gui gui(ts, hs);
    nbErr = gui.run();
    LOG.info<"{} is exiting">(ProgramInfos::APP_SHORT_NAME);
  }
  catch (const UserAskedForHelpException& e) {
    // if user asks for help, he passed -h in the command line
    std::print("{}\n", e.what());
  }
  catch (const PhudException& e) {
    LOG.error(e.what());
    std::print(stderr, "{}\n", e.what());
    ++nbErr;
  }
  catch (const std::logic_error& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    std::print(stderr, "{}\n", e.what());
    ++nbErr;
  }
  catch (const std::exception& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    std::print(stderr, "{}\n", e.what());
    ++nbErr;
  }
  catch (...) {
    LOG.error<"Unknown exception occurred.">();
    std::print(stderr, "Unknown exception occurred.\n");
    ++nbErr;
  }

  return nbErr;
}
