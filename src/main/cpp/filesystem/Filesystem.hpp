#pragma once

#include "containers/Span.hpp"
#include "containers/Vector.hpp"
#include "strings/String.hpp" // String, StringView
#include <filesystem>

using FileTime = std::filesystem::file_time_type;
using Path = std::filesystem::path;

namespace phud::filesystem {
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

[[nodiscard]] String readToString(const Path& p);
String readToString(auto p) = delete; // use only path

/**
 * Returns the list of files ending by .txt but not _summary.txt contained in @param dir.
 */
[[nodiscard]] Vector<Path> listTxtFilesInDir(const Path& dir);
inline bool listTxtFilesInDir(auto p) = delete; // use only path

/**
 * Returns the list of files ending by @param postFix contained in @param dir.
 */
[[nodiscard]] Vector<Path> listFilesInDir(const Path& dir, StringView postFix);
inline bool listFilesInDir(auto p) = delete; // use only path

/**
 * Returns the list of directories contained in @param dir.
 */
[[nodiscard]] Vector<Path> listSubDirs(const Path& dir);
Vector<Path> listSubDirs(auto p) = delete; // use only path

[[nodiscard]] Vector<Path> listFilesAndDirs(const Path& dir);
Vector<Path> listFilesAndDirs(auto p) = delete; // use only path

[[nodiscard]] Vector<Path> listRecursiveFiles(const Path& dir);
Vector<Path> listRecursiveFiles(auto p) = delete; // use only path

template<typename T> requires(std::same_as<T, Path>)
[[nodiscard]] inline Path absolute(const T& file) { return std::filesystem::absolute(file); }

[[nodiscard]] String toString(const FileTime& ft);

[[nodiscard]] bool containsAFileEndingWith(Span<const Path> files, StringView str);

struct [[nodiscard]] PathComparator final {
  bool operator()(const Path& a, const Path& b) const { return a.string() < b.string(); }
};
}; // phud::filesystem
