#include "db/Database.hpp" // std::span
#include "gui/Gui.hpp"
#include "gui/HistoryService.hpp"
#include "gui/TableService.hpp"
#include "language/limits.hpp" // toSizeT
#include "log/Logger.hpp"      // CURRENT_FILE_NAME
#include "constants/ProgramInfos.hpp"
#include "phud/ProgramConfiguration.hpp"
#include "phud/ProgramArguments.hpp" // ProgramArgumentsException, UserAskedForHelpException

#if defined(_WIN32)
#  include <Windows.h> // WinMain, AttachConsole, FILE
#endif                 // _WIN32

// no 'import std': with gcc, mixing it with the standard headers included by the phud headers
// defines some symbols twice (link error)
#include <clocale>   // std::setlocale(), LC_ALL
#include <csignal>   // std::signal(), SIG_DFL, SIGABRT
#include <cstdio>    // stderr, freopen_s()
#include <exception> // std::exception
#include <iostream>  // std::cout, std::cerr, std::cin, std::ios
#include <print>     // std::print(), std::println()
#include <span>
#include <sstream>   // std::ostringstream
#include <stdexcept> // std::logic_error

// TODO: padding: in a class, members must be ordered by type size, the biggest in
//                1st position
// TODO: inclure le siteName dans les id
// TODO: tester toutes les sauvegardes en base
// TODO: utiliser std::source_location pour contextualiser les messages d'exception
//       cf. https://stackoverflow.com/questions/597078/file-line-and-function-usage-in-c
// TODO : détecteur d'historique pour que l'utilisateur sache quoi choisir
// TODO : liste des historiques surveillés
// TODO : le montant gagné par un joueur sur une main doit être enregistré en BDD
// GOOD PRACTISE: apply https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
// GOOD PRACTISE: make internal functions static
// GOOD PRACTISE: use assert, static_assert
// GOOD PRACTISE: ensure the required headers are included without relying on a specific STL
// implementation GOOD PRACTISE: pour tous les constructeurs, vérifier que tous les attributs sont
// initialisés
//                dans le constructeur ou dans la déclaration de classe.
// GOOD PRACTISE: use [[nodiscard]] for all pure functions, or where return value must be examined
// GOOD PRACTISE: no const class attribute
// GOOD PRACTISE: explicit for constructors callable with one parameter
// FUTURE : try unity build
// FUTURE add code documentation (Doxygen, compatible with Javadoc format)
// BUG : crash à la sélection de la table : impossible de trouver l'historique pour table "Wichita
// 05 / 0,01-0,02 NL Holdem / Argent fictif" BUG : au 2eme choix d'historique

static constexpr Logger LOG {CURRENT_FILE_NAME};

static void logErrorAndAbort(int signum) {
  std::signal(signum, SIG_DFL);
  std::ostringstream oss;
  oss << boost::stacktrace::stacktrace();
  std::println(stderr, "{}", oss.str());
  LOG.critical(oss.str());
  std::raise(SIGABRT);
}

#if defined(_WIN32)
/**
 * Under Windows, if the app was launched in a console, do not create a new one.
 */
class [[nodiscard]] ConditionalConsole final {
private:
  bool m_hasConsole = false;

public:
  ConditionalConsole()
    : m_hasConsole {TRUE == AttachConsole(ATTACH_PARENT_PROCESS)} {
    if (m_hasConsole) {
      // attach to parent console
      FILE* fp = nullptr;
      freopen_s(&fp, "CONOUT$", "w", stdout);
      freopen_s(&fp, "CONOUT$", "w", stderr);
      freopen_s(&fp, "CONIN$", "r", stdin);
      std::ios::sync_with_stdio(true);
      std::cout.clear();
      std::cerr.clear();
      std::cin.clear();
      std::print(stderr, "\n");
    }
  }

  ConditionalConsole(const ConditionalConsole&) = delete;
  ConditionalConsole(ConditionalConsole&&) = delete;
  ConditionalConsole& operator=(const ConditionalConsole&) = delete;
  ConditionalConsole& operator=(ConditionalConsole&&) = delete;

  ~ConditionalConsole() {
    if (m_hasConsole) {
      FreeConsole();
    }
  }
}; // class ConditionalConsole

/*
 * Under Windows, graphical user interfaces use 'WinMain' as a default entry point.
 * Note : main or WinMain functions can't be static or [[nodiscard]]
 */
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT) {
  const auto argc = __argc;
  const auto argv = __argv;
  ConditionalConsole attachToParentConsoleIfNeeded;
#else
int main(int argc, const char* const* const argv) {
#endif // _WIN32

  std::setlocale(LC_ALL, "en_US.utf8");
  auto nbErr = 0;

  try {
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

    const std::span args = {argv, argv + argc};

#ifdef __clang__
#  pragma clang diagnostic pop
#endif

    const auto& [loggingLevel, loggingPattern] = ProgramConfiguration::readConfiguration(args);
    // start the logging framework
    LoggingConfig _(loggingPattern);
    Logger::setLoggingLevel(loggingLevel);
    // log an error when the program crashes
    std::signal(SIGSEGV, logErrorAndAbort);
    std::signal(SIGABRT, logErrorAndAbort);
    std::signal(SIGINT, logErrorAndAbort);
    LOG.info<"{} is starting">(ProgramInfos::APP_SHORT_NAME);
    LOG.info<"phud configuration:\n  loggingLevel={}\n  loggingPattern={}">(toString(loggingLevel),
                                                                            loggingPattern);
    Database db(ProgramInfos::DATABASE_NAME);
    TableService tableService(db);
    HistoryService handService(db);
    Gui gui(tableService, handService);
    nbErr = gui.run();
    LOG.info<"{} is exiting">(ProgramInfos::APP_SHORT_NAME);
  } catch (const UserAskedForHelpException& e) {
    // the logging framework is not initialized yet
    // if user asks for help, this is not an error
    std::println("{}", e.what());
  } catch (const ProgramArgumentsException& e) {
    // the logging framework is not initialized yet
    std::println(stderr, "{}", e.what());
    ++nbErr;
  } catch (const PhudException& e) {
    // now the logging framework is initialized
    LOG.error(e.what());
    std::println(stderr, "{}", e.what());
    ++nbErr;
  } catch (const std::logic_error& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    std::println(stderr, "{}", e.what());
    ++nbErr;
  } catch (const std::exception& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    std::println(stderr, "{}", e.what());
    ++nbErr;
  } catch (...) {
    LOG.error<"Unknown exception occurred.">();
    std::println(stderr, "Unknown exception occurred.\n");
    ++nbErr;
  }

  return nbErr;
}
