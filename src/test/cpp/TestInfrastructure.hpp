#pragma once

#include "filesystem/Filesystem.hpp"  // Path, fs::*, String

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4619)
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
[[nodiscard]] Path loadDatabaseFromTestResources(StringView file, StringView pokerSite);
[[nodiscard]] Path getFileFromTestResources(std::u8string_view file);
[[nodiscard]] Path getDirFromTestResources(std::u8string_view dir);
[[nodiscard]] Path getFileFromTestResources(StringView file);
[[nodiscard]] Path getDirFromTestResources(StringView dir);
[[nodiscard]] Path getTestResourcesDir();
[[nodiscard]] Path getMainCppDir();
[[nodiscard]] Path getTestCppDir();

/**
 * A file on the disk that is deleted at the end of the scope.
 * If no name given, a random one is chosen.
 */
class [[nodiscard]] TmpFile final {
private:
  String m_file;

public:
  /**
   * Creates a file that that disappears at the end of the scope. This file will be created in the
   * user's current temp dir.
   * @param fileName the name of the file to be used. If empty, a random one will be used.
   */
  explicit TmpFile(StringView fileName = "");
  TmpFile(const TmpFile&) = delete;
  TmpFile(TmpFile&&) = delete;
  TmpFile& operator=(const TmpFile&) = delete;
  TmpFile& operator=(TmpFile&&) = delete;
  ~TmpFile();
  void print(StringView s) const;
  void printLn(StringView s) const;
  [[nodiscard]] Path path() const noexcept { return m_file; }
  [[nodiscard]] Path getParentDir() const { return Path(m_file).parent_path(); }
  [[nodiscard]] String string() const noexcept { return m_file; }
}; // class TmpFile

/**
 * A directory that disappears at the end of the scope. It is created in the user tmp dir.
 * If already exist, will be deleted then re-created.
 */
class [[nodiscard]] TmpDir final {
private:
  Path m_dir;

public:
  explicit TmpDir(StringView dirName);
  ~TmpDir();
  [[nodiscard]] String operator/(StringView file) const;
  [[nodiscard]] String string() const noexcept { return m_dir.string(); }
  [[nodiscard]] Path path() const noexcept { return m_dir; }
}; // class TmpDir

class [[nodiscard]] LogDisabler final {
private:
  LoggingLevel m_beforeDisabling;

public:
  LogDisabler();
  ~LogDisabler();
}; // class LogDisabler

} // namespace phud::test