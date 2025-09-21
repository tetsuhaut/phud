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
#  include <windows.h> // WinMain. must be included before tlhelp32.h
#  include <tlhelp32.h> // TH32CS_SNAPPROCESS
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


#if defined(_WIN32)

struct [[nodiscard]] AutoCloseHandle final {
  HANDLE m_handle;

  AutoCloseHandle(HANDLE h): m_handle { h } {}
  AutoCloseHandle(const AutoCloseHandle&) = delete;
  AutoCloseHandle& operator=(const AutoCloseHandle&) = delete;
  ~AutoCloseHandle() { if (INVALID_HANDLE_VALUE != m_handle) { CloseHandle(m_handle); } }

  constexpr bool isInvalid() const { return INVALID_HANDLE_VALUE == m_handle; }
  operator HANDLE() const { return m_handle; }
};

static DWORD getParentProcessId(DWORD pid) {
  const auto hSnapshot { AutoCloseHandle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) };
  if (hSnapshot.isInvalid()) { return 0; }  
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);
  DWORD parentPid = 0;
  if (Process32First(hSnapshot, &pe32)) {
    do {
      if (pe32.th32ProcessID == pid) {
        parentPid = pe32.th32ParentProcessID;
        break;
      }
    } while (Process32Next(hSnapshot, &pe32));
  }
  return parentPid;
}

static std::string getProcessName(DWORD pid) {
  const auto hProcess { AutoCloseHandle(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid)) };
  if (hProcess.isInvalid()) { return ""; }
  char processName[MAX_PATH] {'\0'};
  DWORD size = sizeof(processName);
  std::string name;
  if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
    name = processName;
    // Extraire juste le nom du fichier
    if (const auto lastSlash { name.find_last_of("\\/") }; lastSlash != std::string::npos) {
      name = name.substr(lastSlash + 1);
    }
  }
  return name;
}

class [[nodiscard]] ConsoleHider final {
private:
  bool isInConsole() {
    const auto currentPid { GetCurrentProcessId() };
    const auto parentPid { getParentProcessId(currentPid) };
    return (parentPid == 0 or getProcessName(parentPid).find("explorer.exe") == std::string::npos);
  }
public:
  bool m_isInConsole;
  ConsoleHider() : m_isInConsole(isInConsole()) {
    if (!m_isInConsole) { FreeConsole(); }
  }
};

/*
 * Under Windows, graphical user interfaces use 'WinMain' as a default entry point.
 * Note : main or WinMain functions can't be static or inline or [[nodiscard]]
 */
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT) {
  const auto& argc { __argc };
  const auto& argv { __argv };
  ConsoleHider console;
#  else
int main(int argc, const char* const* const argv) {
#endif  // _WIN32
  std::setlocale(LC_ALL, "en_US.utf8");
  std::signal(SIGSEGV, logErrorAndAbort);
  std::signal(SIGABRT, logErrorAndAbort);
  std::signal(SIGINT, logErrorAndAbort);
  LoggingConfig _;
  LOG.info<"{} is starting">(ProgramInfos::APP_SHORT_NAME);
  LOG.info<"m_isInConsole={}">(console.m_isInConsole);
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
    // if user asks for help, he passed -h in the command line
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
