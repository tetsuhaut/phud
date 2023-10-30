#include "containers/algorithms.hpp"
#include "filesystem/Filesystem.hpp" // std::filesystem::path, std::string_view, std::vector
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp"

#include <gsl/gsl>

#include <chrono> // to_time_t
#include <cstring> // std::strerror, strerror_s
#include <ctime> // localtime
#include <fstream> // std::ifstream
#include <iomanip> // std::get_time
#include <system_error>

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;
namespace pa = phud::algorithms;
namespace pf = phud::filesystem;

namespace {
template<typename Iterator>
class [[nodiscard]] FilesInDir final {
private:
  fs::path m_startDir;

public:
  explicit FilesInDir(const fs::path& startDir) : m_startDir{ startDir } {}
  FilesInDir(auto) = delete; // use only std::filesystem::path
  [[nodiscard]] Iterator begin() const { return Iterator(m_startDir); }
  [[nodiscard]] Iterator end() const noexcept { return Iterator(); }
  [[nodiscard]] Iterator begin() { return Iterator(m_startDir); }
  [[nodiscard]] Iterator end() noexcept { return Iterator(); }
}; // class FilesInDir

using DirIt = FilesInDir<fs::directory_iterator>;
using RecursDirIt = FilesInDir<fs::recursive_directory_iterator>;

}; // namespace

// use std::filesystem::path as std needs it
std::string phud::filesystem::readToString(const fs::path& p) {
  phudAssert(!phud::filesystem::isDir(p), "given a dir instead of a file");
  std::string s{ fmt::format("given a non exiting file '{}'", p.string()) };
  phudAssert(phud::filesystem::isFile(p), s.c_str());
  std::ifstream in { p };
  // decltype(std::ifstream::gcount()) is std::streamsize, which is signed.
  // std::string constructor takes a std::string::size_type, which is unsigned.
  // we know that std::ifstream::gcount() is always positive
  in.ignore(std::numeric_limits<std::streamsize>::max());
  std::string result(gsl::narrow_cast<std::string::size_type>(in.gcount()), '\0');
  in.seekg(0);
  in.read(result.data(), gsl::narrow_cast<std::streamsize>(result.size()));
  return result;
}

template<typename DIRECTORY_ITERATOR>
static inline std::vector<fs::path> iterateDirs(const fs::path& dir) {
  std::vector<fs::path> ret;

  if (!phud::filesystem::isDir(dir)) { return ret; }

  for (const auto& dirEntry : DIRECTORY_ITERATOR(dir)) { ret.push_back(dirEntry.path()); }

  return ret;
}
static inline std::vector<fs::path> genericListDirs(auto) = delete; // use only std::filesystem::path

std::vector<fs::path> phud::filesystem::listFilesAndDirs(const fs::path& dir) {
  return iterateDirs<DirIt>(dir);
}

std::vector<fs::path> phud::filesystem::listFilesInDir(const fs::path& dir, std::string_view postFix) {
  std::vector<fs::path> ret;

  if (!phud::filesystem::isDir(dir)) { return ret; }

  for (const auto& dirEntry : fs::directory_iterator(dir)) {
    const auto& entryPath { dirEntry.path() };

    if (entryPath.filename().string().starts_with("20") // like the year 2022
        and entryPath.string().ends_with(postFix)
        and dirEntry.is_regular_file()) {
      ret.push_back(entryPath);
    }
  }

  return ret;
}

std::vector<fs::path> phud::filesystem::listTxtFilesInDir(const fs::path& dir) {
  auto allFilesAndDirs { listFilesAndDirs(dir) };
  std::erase_if(allFilesAndDirs, [](const auto & p) {
    const auto& pstr { p.string() };
    return !isFile(p) or !pstr.ends_with(".txt") or pstr.ends_with("_summary.txt");
  });
  return allFilesAndDirs;
}

std::vector<fs::path> phud::filesystem::listSubDirs(const fs::path& dir) {
  auto allFilesAndDirs { listFilesAndDirs(dir) };
  std::erase_if(allFilesAndDirs, [](const auto & p) noexcept {
    return isFile(p);
  });
  return allFilesAndDirs;
}

std::vector<fs::path> phud::filesystem::listRecursiveFiles(const fs::path& dir) {
  phudAssert(isDir(dir), "given invalid dir");
  std::vector<fs::path> ret;

  for (const auto& dirEntry : RecursDirIt(dir)) {
    if (dirEntry.is_regular_file()) { ret.push_back(dirEntry.path()); }
  }

  return ret;
}

/*[[nodiscard]]*/ std::string phud::filesystem::toString(const std::filesystem::file_time_type& fileTime) {
  namespace sc = std::chrono;
  const auto& now { fileTime - std::filesystem::file_time_type::clock::now() + sc::system_clock::now() };
  const auto& systemClockTimePoint { sc::time_point_cast<sc::system_clock::duration>(now) };
  const auto& posixTime { sc::system_clock::to_time_t(systemClockTimePoint) };
  std::ostringstream oss;
#if defined(_MSC_VER) // use localtime_s instead of std::localtime with _MSC_VER
  std::tm calendarDateTime { .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 0, .tm_mon = 0, .tm_year = 0, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

  if (const auto errorCode { localtime_s(&calendarDateTime, &posixTime) }; 0 !=
      errorCode) [[unlikely]] {
    char msg[256] { 0 };
    strerror_s(msg, std::size(msg), errorCode);
    LOG.error(msg);
    return "";
  }
  oss << std::put_time(&calendarDateTime, "%Y/%m/%d %H:%M:%S");
#else
  oss << std::put_time(std::localtime(&posixTime), "%Y/%m/%d %H:%M:%S");
#endif
  return oss.str();
}

/* [[nodiscard]] */ bool phud::filesystem::containsAFileEndingWith(std::span<const fs::path> files,
    std::string_view str) {
  return pa::anyOf(files, [str](const auto & p) { return p.string().ends_with(str); });
}