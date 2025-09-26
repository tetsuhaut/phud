#include "filesystem/FileUtils.hpp" // std::filesystem::path, std::string_view, std::vector
#include "language/Validator.hpp" // validation::
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include <gsl/gsl> // std::streamsize
#include <chrono> // to_time_t
#include <cstring> // std::strerror, strerror_s
#include <ctime> // localtime
#include <fstream> // std::ifstream
#include <iomanip> // std::get_time

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

namespace {
  template <typename Iterator>
  class [[nodiscard]] FilesInDir final {
  private:
    fs::path m_startDir {};

  public:
    explicit FilesInDir(fs::path startDir) : m_startDir { std::move(startDir) } {}
    explicit FilesInDir(auto) = delete; // use only std::filesystem::path
    [[nodiscard]] Iterator begin() const { return Iterator(m_startDir); }
    [[nodiscard]] Iterator end() const noexcept { return Iterator(); }
    [[nodiscard]] Iterator begin() { return Iterator(m_startDir); }
    [[nodiscard]] Iterator end() noexcept { return Iterator(); }
  }; // class FilesInDir

  using DirIt = FilesInDir<fs::directory_iterator>;
  using RecursDirIt = FilesInDir<fs::recursive_directory_iterator>;
} // anonymous namespace

// use std::filesystem::path as std needs it
std::string phud::filesystem::readToString(const fs::path& p) {
  validation::require(!phud::filesystem::isDir(p), "given a dir instead of a file");
  const auto& s { fmt::format("given a non exiting file '{}'", p.string()) };
  validation::require(phud::filesystem::isFile(p), s);
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

template <typename DIRECTORY_ITERATOR>
static std::vector<fs::path> iterateDirs(const fs::path& dir) {
  std::vector<fs::path> ret;

  if (!phud::filesystem::isDir(dir)) { return ret; }

  for (const auto& dirEntry : DIRECTORY_ITERATOR(dir)) { ret.push_back(dirEntry.path()); }

  return ret;
}

static std::vector<fs::path> genericListDirs(auto) = delete; // use only std::filesystem::path

std::vector<fs::path> phud::filesystem::listFilesAndDirs(const fs::path& dir) {
  return iterateDirs<DirIt>(dir);
}

std::vector<fs::path> phud::filesystem::listFilesInDir(const fs::path& dir,
                                                       std::string_view postFix) {
  std::vector<fs::path> ret;

  if (!phud::filesystem::isDir(dir)) { return ret; }

  for (const auto& dirEntry : fs::directory_iterator(dir)) {
    if (const auto& entryPath { dirEntry.path() }; entryPath.filename().string().starts_with("20") // like the year 2022
      and entryPath.string().ends_with(postFix)
      and dirEntry.is_regular_file()) {
      ret.push_back(entryPath);
    }
  }

  return ret;
}

std::vector<fs::path> phud::filesystem::listTxtFilesInDir(const fs::path& dir) {
  auto allFilesAndDirs { listFilesAndDirs(dir) };
  std::erase_if(allFilesAndDirs, [](const auto& p) {
    const auto& pstr { p.string() };
    return !isFile(p) or !pstr.ends_with(".txt") or pstr.ends_with("_summary.txt");
  });
  return allFilesAndDirs;
}

std::vector<fs::path> phud::filesystem::listSubDirs(const fs::path& dir) {
  auto allFilesAndDirs { listFilesAndDirs(dir) };
  std::erase_if(allFilesAndDirs, [](const auto& p) noexcept {
    return isFile(p);
  });
  return allFilesAndDirs;
}

std::vector<fs::path> phud::filesystem::listRecursiveFiles(const fs::path& dir) {
  validation::require(isDir(dir), "given invalid dir");
  std::vector<fs::path> ret;

  for (const auto& dirEntry : RecursDirIt(dir)) {
    if (dirEntry.is_regular_file()) { ret.push_back(dirEntry.path()); }
  }

  return ret;
}

/*[[nodiscard]]*/
std::string phud::filesystem::toString(const fs::file_time_type& ft) {
  namespace sc = std::chrono;
  const auto& now { ft - fs::file_time_type::clock::now() + sc::system_clock::now() };
  const auto& systemClockTimePoint { sc::time_point_cast<sc::system_clock::duration>(now) };
  const auto& posixTime { sc::system_clock::to_time_t(systemClockTimePoint) };
  std::ostringstream oss;
#if defined(_MSC_VER) // use localtime_s instead of std::localtime with _MSC_VER
  std::tm calendarDateTime {
    .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 0, .tm_mon = 0, .tm_year = 0, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0
  };

  if (const auto errorCode { localtime_s(&calendarDateTime, &posixTime) }; 0 != errorCode) [[unlikely]] {
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

/* [[nodiscard]] */
bool phud::filesystem::containsAFileEndingWith(std::span<const fs::path> files,
                                               std::string_view str) {
  return std::ranges::any_of(files, [str](const auto& p) { return p.string().ends_with(str); });
}

namespace {
  /**
   * Split pattern by '*' to get literal parts.
   */
  std::vector<std::string_view> splitPatternByWildcards(std::string_view pattern) {
    std::vector<std::string_view> patternParts;
    std::size_t start { 0 };

    for (std::size_t pos { 0 }; pos <= pattern.size(); ++pos) {
      if (pos == pattern.size() or pattern[pos] == '*') {
        if (pos > start) {
          patternParts.push_back(pattern.substr(start, pos - start));
        }
        start = pos + 1;
      }
    }

    return patternParts;
  }

  /**
   * Check each pattern part sequentially against filename.
   */
  bool matchPatternParts(std::string_view filename, std::string_view pattern,
                         std::span<const std::string_view> patternParts) {
    std::size_t filenamePos { 0 };

    for (std::size_t i { 0 }; i < patternParts.size(); ++i) {
      const auto& part { patternParts[i] };

      if (i == 0) {
        // First part: must match at start (unless pattern starts with *)
        if (!pattern.starts_with('*')) {
          if (!filename.substr(filenamePos).starts_with(part)) {
            return false;
          }
          filenamePos += part.size();
        }
        else {
          // Find this part anywhere in remaining filename
          const auto foundPos { filename.substr(filenamePos).find(part) };
          if (foundPos == std::string_view::npos) {
            return false;
          }
          filenamePos += foundPos + part.size();
        }
      }
      else if (i == patternParts.size() - 1) {
        // Last part: must match at end (unless pattern ends with *)
        if (!pattern.ends_with('*')) {
          return filename.substr(filenamePos).ends_with(part);
        }
        // Find this part anywhere in remaining filename
        const auto foundPos { filename.substr(filenamePos).find(part) };
        return foundPos != std::string_view::npos;
      }
      else {
        // Middle part: find in remaining filename
        const auto foundPos { filename.substr(filenamePos).find(part) };
        if (foundPos == std::string_view::npos) {
          return false;
        }
        filenamePos += foundPos + part.size();
      }
    }

    return true;
  }

  /**
   * Check if a filename matches a simple wildcard pattern.
   * Supports * as wildcard (matches any sequence of characters).
   */
  bool matchesPattern(std::string_view filename, std::string_view pattern) {
    validation::requireNonEmpty(filename, "filename");
    validation::requireNonEmpty(pattern, "pattern");

    const auto& patternParts { splitPatternByWildcards(pattern) };

    // If no wildcards, must match exactly
    if (patternParts.size() == 1 and pattern.find('*') == std::string_view::npos) {
      return filename == pattern;
    }

    return matchPatternParts(filename, pattern, patternParts);
  }
} // anonymous namespace

std::vector<fs::path> phud::filesystem::listFilesMatchingPattern(const fs::path& dir,
                                                                 std::string_view pattern) {
  validation::requireNonEmpty(pattern, "pattern");
  std::vector<fs::path> ret;

  if (!phud::filesystem::isDir(dir)) { return ret; }

  for (const auto& dirEntry : fs::directory_iterator(dir)) {
    if (dirEntry.is_regular_file()) {
      const auto& filename { dirEntry.path().filename().string() };
      if (!filename.empty()) {
        if (matchesPattern(filename, pattern)) {
          ret.push_back(dirEntry.path());
        }
      }
    }
  }

  return ret;
}
