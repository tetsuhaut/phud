#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace phud::filesystem {
template<typename T> requires(std::same_as<T, std::filesystem::path>)
[[nodiscard]] inline bool isFile(const T& p) noexcept {
  std::error_code ec;
  return std::filesystem::is_regular_file(p, ec) and 0 == ec.value();
}

template<typename T> requires(std::same_as<T, std::filesystem::path>)
[[nodiscard]] inline bool isDir(const T& p) noexcept {
  std::error_code ec;
  return std::filesystem::is_directory(p, ec) and 0 == ec.value();
}

[[nodiscard]] std::string readToString(const std::filesystem::path& p);
std::string readToString(auto p) = delete; // use only path

/**
 * Returns the list of files ending by .txt but not _summary.txt contained in @param dir.
 */
[[nodiscard]] std::vector<std::filesystem::path> listTxtFilesInDir(const std::filesystem::path&
    dir);
inline bool listTxtFilesInDir(auto p) = delete; // use only path

/**
 * Returns the list of files ending by @param postFix contained in @param dir.
 */
[[nodiscard]] std::vector<std::filesystem::path> listFilesInDir(const std::filesystem::path& dir,
    std::string_view postFix);
inline bool listFilesInDir(auto p) = delete; // use only path

/**
 * Returns the list of directories contained in @param dir.
 */
[[nodiscard]] std::vector<std::filesystem::path> listSubDirs(const std::filesystem::path& dir);
std::vector<std::filesystem::path> listSubDirs(auto p) = delete; // use only path

[[nodiscard]] std::vector<std::filesystem::path> listFilesAndDirs(const std::filesystem::path& dir);
std::vector<std::filesystem::path> listFilesAndDirs(auto p) = delete; // use only path

[[nodiscard]] std::vector<std::filesystem::path> listRecursiveFiles(const std::filesystem::path&
    dir);
std::vector<std::filesystem::path> listRecursiveFiles(auto p) = delete; // use only path

template<typename T> requires(std::same_as<T, std::filesystem::path>)
[[nodiscard]] inline std::filesystem::path absolute(const T& file) { return std::filesystem::absolute(file); }

[[nodiscard]] std::string toString(const std::filesystem::file_time_type& ft);

[[nodiscard]] bool containsAFileEndingWith(std::span<const std::filesystem::path> files,
    std::string_view str);

struct [[nodiscard]] PathComparator final {
  bool operator()(const std::filesystem::path& a, const std::filesystem::path& b) const { return a.string() < b.string(); }
};
}; // phud::filesystem
