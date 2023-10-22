#include "containers/algorithms.hpp"
#include "filesystem/Filesystem.hpp" // Path, StringView, Vector
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp"
#include "system/ErrorCode.hpp" // isOk
#include <chrono> // to_time_t
#include <cstring> // std::strerror, strerror_s
#include <ctime> // localtime
#include <fstream> // std::ifstream
#include <gsl/gsl>
#include <iomanip> // std::get_time

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;
namespace pa = phud::algorithms;

namespace {
template<typename Iterator>
class [[nodiscard]] FilesInDir final {
private:
  Path m_startDir;

public:
  template<typename T> requires(std::same_as<T, Path>) // use only Path
  explicit FilesInDir(const T& startDir) : m_startDir{ startDir } {}
  //FilesInDir(auto) = delete; // use only Path
  [[nodiscard]] Iterator begin() const { return Iterator(m_startDir); }
  [[nodiscard]] Iterator end() const noexcept { return Iterator(); }
  [[nodiscard]] Iterator begin() { return Iterator(m_startDir); }
  [[nodiscard]] Iterator end() noexcept { return Iterator(); }
}; // class FilesInDir

using DirIt = FilesInDir<fs::directory_iterator>;
using RecursDirIt = FilesInDir<fs::recursive_directory_iterator>;

}; // namespace

// use Path as std needs std::path
String phud::filesystem::readToString(const Path& p) {
  phudAssert(!phud::filesystem::isDir(p), "given a dir instead of a file");
  String s{ fmt::format("given a non exiting file '{}'", p.string()) };
  phudAssert(phud::filesystem::isFile(p), s.c_str());
  std::ifstream in { p };
  // decltype(std::ifstream::gcount()) is std::streamsize, which is signed.
  // String constructor takes a String::size_type, which is unsigned.
  // we know that std::ifstream::gcount() is always positive
  in.ignore(std::numeric_limits<std::streamsize>::max());
  String result(gsl::narrow_cast<String::size_type>(in.gcount()), '\0');
  in.seekg(0);
  in.read(result.data(), gsl::narrow_cast<std::streamsize>(result.size()));
  return result;
}

template<typename DIRECTORY_ITERATOR>
static inline Vector<Path> iterateDirs(const Path& dir) {
  Vector<Path> ret;

  if (!phud::filesystem::isDir(dir)) { return ret; }

  for (const auto& dirEntry : DIRECTORY_ITERATOR(dir)) { ret.push_back(dirEntry.path()); }

  return ret;
}
static inline Vector<Path> genericListDirs(auto) = delete; // use only Path

Vector<Path> phud::filesystem::listFilesAndDirs(const Path& dir) {
  return iterateDirs<DirIt>(dir);
}

Vector<Path> phud::filesystem::listFilesInDir(const Path& dir, StringView postFix) {
  Vector<Path> ret;

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

Vector<Path> phud::filesystem::listTxtFilesInDir(const Path& dir) {
  auto allFilesAndDirs { listFilesAndDirs(dir) };
  std::erase_if(allFilesAndDirs, [](const auto & p) {
    const auto& pstr { p.string() };
    return !isFile(p) or !pstr.ends_with(".txt") or pstr.ends_with("_summary.txt");
  });
  return allFilesAndDirs;
}

Vector<Path> phud::filesystem::listSubDirs(const Path& dir) {
  auto allFilesAndDirs { listFilesAndDirs(dir) };
  std::erase_if(allFilesAndDirs, [](const auto & p) noexcept {
    return isFile(p);
  });
  return allFilesAndDirs;
}

Vector<Path> phud::filesystem::listRecursiveFiles(const Path& dir) {
  phudAssert(isDir(dir), "given invalid dir");
  Vector<Path> ret;

  for (const auto& dirEntry : RecursDirIt(dir)) {
    if (dirEntry.is_regular_file()) { ret.push_back(dirEntry.path()); }
  }

  return ret;
}

/*[[nodiscard]]*/ String phud::filesystem::toString(const FileTime& fileTime) {
  namespace sc = std::chrono;
  const auto& now { fileTime - FileTime::clock::now() + sc::system_clock::now() };
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

/* [[nodiscard]] */ bool phud::filesystem::containsAFileEndingWith(Span<const Path> files,
    StringView str) {
  return pa::anyOf(files, [str](const auto & p) { return p.string().ends_with(str); });
}