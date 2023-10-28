#pragma once

#include "filesystem/Filesystem.hpp"  // Path, fs::*, String

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4619 26495)
#elif defined(__MINGW32__) // removal of specific gcc warnings due to FLTK
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Weffc++"
#endif  // _MSC_VER

#include <boost/test/unit_test.hpp>

#if defined(_MSC_VER)  // end of specific msvc warnings removal
#  pragma warning(pop)
#elif defined(__MINGW32__)
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

/* forward declaration */
enum class LoggingLevel : short;

namespace phud::test {
/**
 * @return the absolute path of src/test/resources.
 * @note We use Path to mix UTF-8 and UTF-16 file names.
*/
[[nodiscard]] phud::filesystem::Path loadDatabaseFromTestResources(std::string_view file, std::string_view pokerSite);
[[nodiscard]] phud::filesystem::Path getFileFromTestResources(std::u8string_view file);
[[nodiscard]] phud::filesystem::Path getDirFromTestResources(std::u8string_view dir);
[[nodiscard]] phud::filesystem::Path getFileFromTestResources(std::string_view file);
[[nodiscard]] phud::filesystem::Path getDirFromTestResources(std::string_view dir);
[[nodiscard]] phud::filesystem::Path getTestResourcesDir();
[[nodiscard]] phud::filesystem::Path getMainCppDir();
[[nodiscard]] phud::filesystem::Path getTestCppDir();

/**
 * A file on the disk that is deleted at the end of the scope.
 * If no name given, a random one is chosen.
 */
class [[nodiscard]] TmpFile final {
private:
  std::string m_file;

public:
  /**
   * Creates a file that that disappears at the end of the scope. This file will be created in the
   * user's current temp dir.
   * @param fileName the name of the file to be used. If empty, a random one will be used.
   */
  explicit TmpFile(std::string_view fileName = "");
  TmpFile(const TmpFile&) = delete;
  TmpFile(TmpFile&&) = delete;
  TmpFile& operator=(const TmpFile&) = delete;
  TmpFile& operator=(TmpFile&&) = delete;
  ~TmpFile();
  void print(std::string_view s) const;
  void printLn(std::string_view s) const;
  [[nodiscard]] phud::filesystem::Path path() const noexcept { return m_file; }
  [[nodiscard]] phud::filesystem::Path getParentDir() const { return phud::filesystem::Path(m_file).parent_path(); }
  [[nodiscard]] std::string string() const noexcept { return m_file; }
}; // class TmpFile

/**
 * A directory that disappears at the end of the scope. It is created in the user tmp dir.
 * If already exist, will be deleted then re-created.
 */
class [[nodiscard]] TmpDir final {
private:
  phud::filesystem::Path m_dir;

public:
  explicit TmpDir(std::string_view dirName);
  ~TmpDir();
  [[nodiscard]] std::string operator/(std::string_view file) const;
  [[nodiscard]] std::string string() const noexcept { return m_dir.string(); }
  [[nodiscard]] phud::filesystem::Path path() const noexcept { return m_dir; }
}; // class TmpDir

class [[nodiscard]] LogDisabler final {
private:
  LoggingLevel m_beforeDisabling;

public:
  LogDisabler();
  ~LogDisabler();
}; // class LogDisabler

} // namespace phud::test