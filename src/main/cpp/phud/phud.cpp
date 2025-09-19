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
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"

#if defined(__MINGW32__) // removal of specific gcc warnings due to Boost
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Weffc++"
#endif  // __MINGW32__

#include <boost/stacktrace.hpp> // as std::stacktrace does not work with gcc 15.2 on Windows

#if defined(__MINGW32__) // end of specific gcc warnings removal
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

#include <csignal> // std::signal(), SIG_DFL, SIGABRT
#include <print>
#include <sstream> // std::ostringstream

#if defined(_WIN32)
#  include <windows.h> // WinMain
#endif  // _WIN32

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
// l'historique est maintenant dans C:\Users\Simon\AppData\Roaming\wam.04351C371E530C3762CBA45FA283ED972DCDEFB6.1\Local Store\documents\accounts\sabre_laser\history
// BUG : au 2eme choix d'historique
// TODO : détecteur d'historique pour que l'utilisateur sache quoi choisir
// TODO : liste des historiques surveillés


static Logger LOG { CURRENT_FILE_NAME };

static inline void logErrorAndAbort(int signum) {
  std::signal(signum, SIG_DFL);
  std::ostringstream oss;
  oss << boost::stacktrace::stacktrace();
  std::print(stderr, "{}\n", oss.str());
  LOG.critical(oss.str());
  std::raise(SIGABRT);
}

struct [[nodiscard]] LoggingConfig final {
  LoggingConfig() { Logger::setupFileInfoLogging("[%D %H:%M:%S.%e] [%l] [%t] %v"); }
  LoggingConfig(const LoggingConfig&) = delete;
  LoggingConfig(LoggingConfig&&) = delete;
  LoggingConfig& operator=(const LoggingConfig&) = delete;
  LoggingConfig& operator=(LoggingConfig&&) = delete;
  ~LoggingConfig() { Logger::shutdownLogging(); }
};

/*
 * Under Windows, graphical user interfaces use 'WinMain' as a default entry point.
 */
#if defined(_WIN32)
/*[[nodiscard]] static inline*/ int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR,
    _In_ INT) {
  const auto& argc { __argc };
  const auto& argv { __argv };
#  else
/*[[nodiscard]] static inline*/ int main(int argc, const char* const* const argv) {
#endif  // _WIN32
  std::setlocale(LC_ALL, "en_US.utf8");
  std::signal(SIGSEGV, logErrorAndAbort);
  std::signal(SIGABRT, logErrorAndAbort);
  std::signal(SIGINT, logErrorAndAbort);
  LoggingConfig _;
  LOG.info<"{} is starting">(ProgramInfos::APP_SHORT_NAME);
  auto nbErr { 0 };

  try {
    auto args { std::span(argv, limits::toSizeT(argc)) };
    const auto& [oHistoDir, loggingLevel] { ProgramConfiguration::readConfiguration(args) };
    Logger::setLoggingLevel(loggingLevel);
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
    }

    Gui gui(ts, hs);
    nbErr = gui.run();
    LOG.info<"{} is exiting">(ProgramInfos::APP_SHORT_NAME);
  } catch (const UserAskedForHelpException& e) {
    LOG.info(e.what());
    std::print("{}\n", e.what());
  } catch (const PhudException& e) {
    LOG.error(e.what());
    std::print(stderr, "{}\n", e.what());
    ++nbErr;
  } catch (const std::logic_error& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    std::print(stderr, "{}\n", e.what());
    ++nbErr;
  } catch (const std::exception& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    std::print(stderr, "{}\n", e.what());
    ++nbErr;
  } catch (...) {
    LOG.error<"Unknown exception occurred.">();
    std::print(stderr, "Unknown exception occurred.\n");
    ++nbErr;
  }

  return nbErr;
}
