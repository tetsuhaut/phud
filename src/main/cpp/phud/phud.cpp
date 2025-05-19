#include "language/limits.hpp" // toSizeT
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/App.hpp"
#include "phud/ProgramArguments.hpp"  // ProgramArgumentsException, UserAskedForHelpException
#include "mainLib/ProgramInfos.hpp"

#if defined(__MINGW32__) // removal of specific gcc warnings due to Boost
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Weffc++"
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#endif  // __MINGW32__

#include <boost/stacktrace.hpp> // boost::stacktrace

#if defined(__MINGW32__) // end of specific gcc warnings removal
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

#include <spdlog/fmt/bundled/printf.h> // fmt::print
#include <csignal> // std::signal()
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
  fmt::print(stderr, "{}\n", oss.str());
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
    const auto& [oPokerSiteHistoryDir, oLoggingLevel] { parseProgramArguments(std::span(argv, limits::toSizeT(argc))) };

    if (oLoggingLevel.has_value()) { Logger::setLoggingLevel(oLoggingLevel.value()); }

    App mainProgram { "phud.db" };

    if (oPokerSiteHistoryDir.has_value()) {
      const auto& winamaxGamesHistoryDir { oPokerSiteHistoryDir.value() };

      if (App::isValidHistory(winamaxGamesHistoryDir)) {
        mainProgram.importHistory(winamaxGamesHistoryDir);
      }
    }

    nbErr = mainProgram.showGui();
    LOG.info<"{} is exiting">(ProgramInfos::APP_SHORT_NAME);
  } catch (const PhudException& e) {
    LOG.error(e.what());
  } catch (const std::logic_error& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    ++nbErr;
  } catch (const std::exception& e) {
    LOG.error<"Unexpected exception: {}">(e.what());
    ++nbErr;
  } catch (...) {
    LOG.error<"Unknown exception occurred.">();
    ++nbErr;
  }

  return nbErr;
}
