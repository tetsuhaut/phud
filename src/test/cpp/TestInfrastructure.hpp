#pragma once

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4365 4619 26426 26495)
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wglobal-constructors"
#  pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif // _MSC_VER

#include <boost/test/unit_test.hpp>

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#elif defined(__clang__)
#  pragma clang diagnostic pop
#endif // _MSC_VER

#if defined(_MSC_VER)
#  pragma warning(disable : 26426 26440 26830 26860)
#endif // _MSC_VER

#include "strings/StringLiteral.hpp"
#include <chrono>
#include <filesystem> // std::filesystem::path
#include <memory>     // std::unique_ptr
#include <string_view>

/* forward declarations */
enum class LoggingLevel : short;
class Database;
class PlayerStatistics;
class Site;

namespace phud::test {
  /**
   * @return the absolute path of src/test/resources.
   * @note We use std::filesystem::path to mix UTF-8 and UTF-16 file names.
   */
  [[nodiscard]] std::filesystem::path loadDatabaseFromTestResources(std::string_view file,
                                                                    std::string_view pokerSite);
  [[nodiscard]] std::filesystem::path getFileFromTestResources(std::u8string_view file);
  [[nodiscard]] std::filesystem::path getDirFromTestResources(std::u8string_view dir);
  [[nodiscard]] std::filesystem::path getFileFromTestResources(std::string_view file);
  [[nodiscard]] std::filesystem::path getDirFromTestResources(std::string_view dir);
  [[nodiscard]] std::filesystem::path getTestResourcesDir();
  [[nodiscard]] std::filesystem::path getMainCppDir();
  [[nodiscard]] std::filesystem::path getTestCppDir();

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
    [[nodiscard]] std::filesystem::path path() const { return m_file; }
    [[nodiscard]] std::filesystem::path getParentDir() const {
      return std::filesystem::path(m_file).parent_path();
    }
    [[nodiscard]] std::string string() const { return m_file; }
  }; // class TmpFile

  /**
   * A directory that disappears at the end of the scope. It is created in the user tmp dir.
   * If already exist, will be deleted then re-created.
   */
  class [[nodiscard]] TmpDir final {
  private:
    std::filesystem::path m_dir;

  public:
    explicit TmpDir(std::string_view dirName);
    TmpDir(const TmpDir&) = delete;
    TmpDir(TmpDir&&) = delete;
    TmpDir& operator=(const TmpDir&) = delete;
    TmpDir& operator=(TmpDir&&) = delete;
    ~TmpDir();
    [[nodiscard]] std::string operator/(std::string_view file) const;
    [[nodiscard]] std::string string() const { return m_dir.string(); }
    [[nodiscard]] std::filesystem::path path() const { return m_dir; }
  }; // class TmpDir

  class [[nodiscard]] LogDisabler final {
  private:
    LoggingLevel m_beforeDisabling;

  public:
    LogDisabler();
    LogDisabler(const LogDisabler&) = delete;
    LogDisabler(LogDisabler&&) = delete;
    LogDisabler& operator=(const LogDisabler&) = delete;
    LogDisabler& operator=(LogDisabler&&) = delete;
    ~LogDisabler();
  }; // class LogDisabler

  [[nodiscard]] bool isSet(const auto& container) {
    auto copy {container};
    std::sort(std::begin(copy), std::end(copy));
    return std::end(copy) == std::adjacent_find(std::begin(copy), std::end(copy));
  }

  /**
   * @returns the site loaded from the given dir of the test resources, saved into an in-memory
   * database. Both are built at the first call only, then shared: they must not be modified.
   * @param historyDir relative to src/test/resources, e.g. "Winamax/preflopStats"
   */
  [[nodiscard]] const Site& getLoadedSite(std::string_view historyDir);
  [[nodiscard]] Database& getLoadedDatabase(std::string_view historyDir);

  /**
   * The history of the given dir of the test resources, loaded and saved into an in-memory
   * database only once for all the tests. It is a named scope, not a fixture: the tests call it
   * with its name, so that the origin of the symbols is visible. Usage:
   *   using PreflopStats = pt::LoadedHistory<"Winamax/preflopStats">;
   *   const auto& stats = PreflopStats::database().readTableStatistics(...);
   * @tparam HISTORY_DIR relative to src/test/resources
   */
  template <StringLiteral HISTORY_DIR>
  struct [[nodiscard]] LoadedHistory final {
    LoadedHistory() = delete;

    [[nodiscard]] static const Site& site() {
      return getLoadedSite(std::string_view(HISTORY_DIR));
    }

    [[nodiscard]] static Database& database() {
      return getLoadedDatabase(std::string_view(HISTORY_DIR));
    }
  }; // struct LoadedHistory

  /**
   * Checks the name, the number of hands, the VPIP and the PFR of the given player statistics.
   */
  void assertStats(const PlayerStatistics* pStats, std::string_view playerName, int nbHands,
                   int vpip, int pfr);

  /**
   * Kills the program if not destroyed before the given period: Boost.Test timeouts don't stop a
   * test that hangs under Windows, they are only checked when the test ends.
   */
  class [[nodiscard]] TimeBomb /*final*/ {
  public:
    static std::unique_ptr<TimeBomb> create(std::chrono::milliseconds countDownToExplosion,
                                            std::string_view testName);
    virtual ~TimeBomb();
  }; // class TimeBomb

} // namespace phud::test
