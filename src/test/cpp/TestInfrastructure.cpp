///////////////////////////////////////////////////////////////////////////////////
// No unit tests here. This file must be the only one defining BOOST_TEST_MODULE. //
///////////////////////////////////////////////////////////////////////////////////
#define BOOST_TEST_MODULE Unit Tests for the Poker Head-Up Dispay

#include "TestInfrastructure.hpp" // boost::unit_test::*
#include "db/Database.hpp"
#include "entities/Game.hpp" // needed as Site declares incomplete CashGame type
#include "entities/Player.hpp" // needed as Site declares incomplete Player type
#include "entities/Site.hpp"
#include "history/WinamaxHistory.hpp" // PokerSiteHistory
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp" // fmt::format(), LoggingLevel
#include "strings/StringUtils.hpp" // phud::strings::*
#include "threads/ThreadPool.hpp"

#include <boost/test/debug.hpp> // detect_memory_leaks()

#if defined(__MINGW32__) // removal of specific gcc warnings
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#endif //__MINGW32__

#include <boost/test/unit_test_parameters.hpp>

#if defined(__MINGW32__)
#  pragma GCC diagnostic pop
#endif //__MINGW32__

#include <fstream> // std::ofstream
#include <source_location>
#include <system_error> // std::error_code

namespace fs = std::filesystem;
namespace pf = phud::filesystem;
namespace ps = phud::strings;
namespace pt = phud::test;

//#define BOOST_TEST_ALTERNATIVE_INIT_API
//int main(int argc, char* argv[], char* /*envp*/[]) {
//  std::setlocale(LC_ALL, "en_US.utf8");
//  auto ret = boost::unit_test::unit_test_main(init_unit_test, argc, argv);
//  return ret;
//}

class [[nodiscard]] GlobalFixture final {
public:
  GlobalFixture() {
    boost::debug::detect_memory_leaks(false);
    Logger::setupConsoleWarnLogging("[%D %H:%M:%S.%e] [%l] [%t] %v");
    namespace but = boost::unit_test;

    if (but::log_level::log_successful_tests ==
        but::runtime_config::get<but::log_level>(but::runtime_config::btrt_log_level)) {
      /* if unit tests are launched with option -l all, set log to trace level */
      Logger::setLoggingLevel(LoggingLevel::trace);
    }
  }
  ~GlobalFixture() {
    Logger::shutdownLogging();
    ThreadPool::stop();
  }
}; /* GlobalFixture */

BOOST_GLOBAL_FIXTURE(GlobalFixture);

namespace {
struct IsFile final {};
struct IsDir final {};
};

template<typename T> requires(std::same_as<T, ::IsFile> or std::same_as<T, ::IsDir>)
[[nodiscard]] static inline fs::path getGenericFileFromTestResources(const auto& file) {
  phudAssert(!file.empty(), "file or dir can't be empty");
  phudAssert('/' != file.front(), "file or dir can't start with '/'");
  const auto& ret { (pt::getTestResourcesDir() / file) };

  if constexpr(std::is_same_v<T, ::IsFile>) {
    if (pf::isFile(ret)) { return ret; }
  }

  if constexpr(std::is_same_v<T, ::IsDir>) {
    if (pf::isDir(ret)) { return ret; }
  }

  const auto sl { std::source_location::current() };
  throw std::runtime_error { fmt::format("{} {} Couldn't find the file '{}' looking into '{}'.",
                                         sl.file_name(), sl.line(),
                                         reinterpret_cast<const char*>(file.data()), // fmt won't handle char8_t
                                         pt::getTestResourcesDir().string()) };
}

fs::path pt::getFileFromTestResources(std::u8string_view file) {
  return getGenericFileFromTestResources<::IsFile>(file);
}

fs::path pt::getDirFromTestResources(std::u8string_view dir) {
  return getGenericFileFromTestResources<::IsDir>(dir);
}

fs::path pt::getFileFromTestResources(std::string_view file) {
  return getGenericFileFromTestResources<::IsFile>(file);
}

fs::path pt::getDirFromTestResources(std::string_view dir) {
  return getGenericFileFromTestResources<::IsDir>(dir);
}

[[nodiscard]] static inline fs::path throwIfNotADirectory(const fs::path& dir,
    std::string_view macro) {
  if (pf::isDir(dir)) { return dir; }

  throw std::runtime_error { fmt::format("Couldn't find the directory '{}' whereas it is the value of the macro '{}'", dir.string(), macro) };
}

fs::path pt::getMainCppDir() {
#ifndef PHUD_MAIN_SRC_DIR
#error The macro PHUD_MAIN_SRC_DIR should have been defined in CMakeLists.txt
#endif // PHUD_MAIN_SRC_DIR
  return throwIfNotADirectory(PHUD_MAIN_SRC_DIR, "PHUD_MAIN_SRC_DIR");
}

fs::path pt::getTestCppDir() {
#ifndef PHUD_TEST_SRC_DIR
#error The macro PHUD_TEST_SRC_DIR should have been defined in CMakeLists.txt
#endif // PHUD_TEST_SRC_DIR
  return throwIfNotADirectory(PHUD_TEST_SRC_DIR, "PHUD_TEST_SRC_DIR");
}

fs::path pt::getTestResourcesDir() {
#ifndef PHUD_TEST_RESOURCE_DIR
#error The macro PHUD_TEST_RESOURCE_DIR should have been defined in CMakeLists.txt
#endif // PHUD_TEST_RESOURCE_DIR
  return throwIfNotADirectory(PHUD_TEST_RESOURCE_DIR, "PHUD_TEST_RESOURCE_DIR");
}

/**
 * @return the absolute path of a temp file.
 * @note We use Path to handle UTF-8 and UTF-16 file names.
*/
[[nodiscard]] static inline fs::path getTmpFilePath() {
  char ret[L_tmpnam] { '\0' };

  if (const auto errorCode { tmpnam_s(ret, std::size(ret)) }; 0 != errorCode) [[unlikely]] {
    strerror_s(ret, std::size(ret), errorCode);
    throw ret;
  }

  // with gcc lhmouse, fs::temp_directory_path() returns an empty path and
  // tmpnam_s gives a relative file name
  if (fs::path(ret).is_relative()) {
    return fs::temp_directory_path() / fs::path(ret).filename();
  }

  return ret;
}

static inline void removeWithMessage(const fs::path& file) {
  const auto& fileType { pf::isFile(file) ? "file" : "directory" };

  if (std::error_code ec; !std::filesystem::remove_all(file, ec)) {
    if (0 == ec.value()) {
      BOOST_TEST_MESSAGE(fmt::format("tried to remove the unexising {} '{}'", fileType, file.string()));
    } else [[unlikely]] {
        BOOST_TEST_MESSAGE(fmt::format("couldn't remove the {} '{}'", fileType, file.string()));
        BOOST_TEST_MESSAGE(ec.message());
      }
    }
}

pt::TmpFile::TmpFile(std::string_view name)
  : m_file { name.empty() ? getTmpFilePath().string() : std::string(name) } {
  std::filesystem::remove(m_file);
  this->print("");
}

pt::TmpFile::~TmpFile() {
  removeWithMessage(m_file);
}

void pt::TmpFile::print(std::string_view s) const {
  std::ofstream writer(m_file, std::ios_base::app);
  writer << s;
}

void pt::TmpFile::printLn(std::string_view s) const {
  std::ofstream writer(m_file, std::ios_base::app);
  writer << s << '\n';
}

pt::TmpDir::TmpDir(std::string_view dirName) :
  m_dir { std::filesystem::temp_directory_path().append(dirName) } {
  if (pf::isDir(m_dir)) { removeWithMessage(m_dir); }

  std::filesystem::create_directories(m_dir);
}

pt::TmpDir::~TmpDir() { removeWithMessage(m_dir); }

std::string pt::TmpDir::operator/(std::string_view file) const {
  fs::path p { m_dir };
  return p.append(file).string();
}

pt::LogDisabler::LogDisabler()
  : m_beforeDisabling { Logger::getCurrentLoggingLevel() } {
  Logger::setLoggingLevel(LoggingLevel::none);
}

pt::LogDisabler::~LogDisabler() { Logger::setLoggingLevel(m_beforeDisabling); }