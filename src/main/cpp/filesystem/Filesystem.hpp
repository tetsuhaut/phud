#pragma once

#include "strings/StringUtils.hpp" // String, std::string_view
#include <filesystem>
#include <span>

namespace phud::filesystem {
  using FileTime = std::filesystem::file_time_type;
  using Path = std::filesystem::path;
// for all those functions, we use Path because std needs std::path
template<typename T> requires(std::same_as<T, Path>)
[[nodiscard]] inline bool isFile(const T& p) noexcept {
  std::error_code ec;
  return std::filesystem::is_regular_file(p, ec) and 0 == ec.value();
}

template<typename T> requires(std::same_as<T, Path>)
[[nodiscard]] inline bool isDir(const T& p) noexcept {
  std::error_code ec;
  return std::filesystem::is_directory(p, ec) and 0 == ec.value();
}

[[nodiscard]] std::string readToString(const Path& p);
std::string readToString(auto p) = delete; // use only path

/**
 * Returns the list of files ending by .txt but not _summary.txt contained in @param dir.
 */
[[nodiscard]] std::vector<Path> listTxtFilesInDir(const Path& dir);
inline bool listTxtFilesInDir(auto p) = delete; // use only path

/**
 * Returns the list of files ending by @param postFix contained in @param dir.
 */
[[nodiscard]] std::vector<Path> listFilesInDir(const Path& dir, std::string_view postFix);
inline bool listFilesInDir(auto p) = delete; // use only path

/**
 * Returns the list of directories contained in @param dir.
 */
[[nodiscard]] std::vector<Path> listSubDirs(const Path& dir);
std::vector<Path> listSubDirs(auto p) = delete; // use only path

[[nodiscard]] std::vector<Path> listFilesAndDirs(const Path& dir);
std::vector<Path> listFilesAndDirs(auto p) = delete; // use only path

[[nodiscard]] std::vector<Path> listRecursiveFiles(const Path& dir);
std::vector<Path> listRecursiveFiles(auto p) = delete; // use only path

template<typename T> requires(std::same_as<T, Path>)
[[nodiscard]] inline Path absolute(const T& file) { return std::filesystem::absolute(file); }

[[nodiscard]] std::string toString(const FileTime& ft);

[[nodiscard]] bool containsAFileEndingWith(std::span<const Path> files, std::string_view str);

struct [[nodiscard]] PathComparator final {
  bool operator()(const Path& a, const Path& b) const { return a.string() < b.string(); }
};
}; // phud::filesystem
