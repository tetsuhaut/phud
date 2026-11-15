#include "TestInfrastructure.hpp"
#include "TimeBomb.hpp"
#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "gui/HistoryService.hpp"
#include "statistics/PlayerStatistics.hpp" // required by TableStatistics
#include "statistics/TableStatistics.hpp"
#include <condition_variable>
#include <mutex>

namespace fs = std::filesystem;
namespace pt = phud::test;

static constexpr std::chrono::milliseconds TB_PERIOD {20000};
static constexpr std::chrono::seconds WAIT_PERIOD {15};
static constexpr std::string_view TESTVILLE = "20260101_Testville_real_holdem_no-limit.txt";
static constexpr std::string_view AUTREVILLE = "20260102_Autreville_real_holdem_no-limit.txt";

namespace {
  /**
   * The names of the history files saved by the HistoryService, filled from another thread.
   */
  class [[nodiscard]] SavedFiles final {
  private:
    std::mutex m_mutex {};
    std::condition_variable m_cv {};
    std::vector<std::string> m_names {};
    bool m_isDone {false};

  public:
    void add(const fs::path& file) {
      const std::scoped_lock lock {m_mutex};
      m_names.push_back(file.filename().string());
      m_cv.notify_all();
    }

    void done() {
      const std::scoped_lock lock {m_mutex};
      m_isDone = true;
      m_cv.notify_all();
    }

    [[nodiscard]] std::vector<std::string> waitUntilDone() {
      std::unique_lock lock {m_mutex};
      m_cv.wait_for(lock, WAIT_PERIOD, [this]() { return m_isDone; });
      return m_names;
    }

    [[nodiscard]] std::vector<std::string> waitUntilOneFile() {
      std::unique_lock lock {m_mutex};
      m_cv.wait_for(lock, WAIT_PERIOD, [this]() { return !m_names.empty(); });
      return m_names;
    }
  }; // class SavedFiles

  /**
   * Copies the Winamax/preflopStats history into the given dir, which can then be modified.
   */
  void copyHistory(const pt::TmpDir& dir) {
    const auto source = pt::getDirFromTestResources("Winamax/preflopStats/history");
    const auto target = dir.path() / "history";
    fs::create_directories(target);
    fs::copy(source, target, fs::copy_options::overwrite_existing);
  }

  [[nodiscard]] bool hasStats(const Database& db, std::string_view table) {
    return db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, table).isValid();
  }
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(HistoryServiceTest)

BOOST_AUTO_TEST_CASE(HistoryServiceTest_catchUpShouldImportOnlyTheFilesModifiedSince) {
  const auto tb = TimeBomb::create(TB_PERIOD, "HistoryServiceTest_catchUp");
  const pt::TmpDir dir("HistoryServiceTest_catchUp");
  copyHistory(dir);
  const auto now = fs::file_time_type::clock::now();
  fs::last_write_time(dir.path() / "history" / TESTVILLE, now);
  fs::last_write_time(dir.path() / "history" / AUTREVILLE, now - std::chrono::hours(2));
  Database db;
  HistoryService service {db};
  SavedFiles savedFiles;
  service.importFilesModifiedSince(
      dir.path(), std::chrono::system_clock::now() - std::chrono::hours(1),
      [&savedFiles](const fs::path& f) { savedFiles.add(f); },
      [&savedFiles]() { savedFiles.done(); });
  const auto names = savedFiles.waitUntilDone();
  BOOST_REQUIRE(1 == names.size());
  BOOST_TEST(TESTVILLE == names.front());
  BOOST_TEST(hasStats(db, "Testville"));
  BOOST_TEST(!hasStats(db, "Autreville"));
}

BOOST_AUTO_TEST_CASE(HistoryServiceTest_importShouldSaveEveryFile) {
  const auto tb = TimeBomb::create(TB_PERIOD, "HistoryServiceTest_import");
  const pt::TmpDir dir("HistoryServiceTest_import");
  copyHistory(dir);
  Database db;
  HistoryService service {db};
  SavedFiles progress;
  std::size_t nbFiles {0};
  service.importHistory(
      dir.path(), [&progress]() { progress.add("one more file"); },
      [&nbFiles](std::size_t nb) { nbFiles = nb; }, [&progress]() { progress.done(); });
  const auto names = progress.waitUntilDone();
  BOOST_TEST(2 == nbFiles);
  BOOST_TEST(2 == names.size());
  BOOST_TEST(hasStats(db, "Testville"));
  BOOST_TEST(hasStats(db, "Autreville"));
}

BOOST_AUTO_TEST_CASE(HistoryServiceTest_watchingShouldSaveAModifiedFile) {
  const auto tb = TimeBomb::create(TB_PERIOD, "HistoryServiceTest_watching");
  const pt::TmpDir dir("HistoryServiceTest_watching");
  copyHistory(dir);
  Database db;
  HistoryService service {db};
  SavedFiles savedFiles;
  service.watchHistory(dir.path(), [&savedFiles](const fs::path& f) { savedFiles.add(f); });
  BOOST_REQUIRE(!hasStats(db, "Testville"));
  // as Winamax does after each hand
  fs::copy_file(pt::getDirFromTestResources("Winamax/preflopStats/history") / TESTVILLE,
                dir.path() / "history" / TESTVILLE, fs::copy_options::overwrite_existing);
  fs::last_write_time(dir.path() / "history" / TESTVILLE, fs::file_time_type::clock::now());
  const auto names = savedFiles.waitUntilOneFile();
  service.stopWatchingHistory();
  BOOST_REQUIRE(!names.empty());
  BOOST_TEST(TESTVILLE == names.front());
  BOOST_TEST(hasStats(db, "Testville"));
}

BOOST_AUTO_TEST_SUITE_END()
