#include "TestInfrastructure.hpp"
#include "db/Database.hpp"
#include "gui/TableService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "statistics/PlayerStatistics.hpp" // required by TableStatistics
#include "statistics/TableStatistics.hpp"

namespace fs = std::filesystem;
namespace pt = phud::test;

using PreflopStats = pt::LoadedHistory<"Winamax/preflopStats">;

namespace {
  constexpr std::string_view TESTVILLE_TITLE = "Winamax Testville";

  /**
   * A TableService using the database containing the Winamax/preflopStats history. Each test
   * creates its own one, so that the origin of its symbols is visible: f.service, f.follow()...
   */
  struct [[nodiscard]] Fixture final {
    fs::path dir {pt::getDirFromTestResources("Winamax/preflopStats")};
    TableService service {PreflopStats::database()};
    std::vector<std::string> notifiedTables {};

    Fixture() { service.setPokerSiteHistory(PokerSiteHistory::newInstance(dir)); }

    [[nodiscard]] TableService::TableObserverCallback observer() {
      return [this](TableStatistics&& ts) {
        notifiedTables.emplace_back(ts.getTable());
      };
    }

    void saved(std::string_view fileName) {
      service.onHistoryFileSaved(dir / "history" / fileName);
    }

    /**
     * Follows the given Poker table, i.e. listens to its history updates, and forgets the
     * notification of the statistics already known.
     */
    void follow(std::string_view tableWindowTitle) {
      BOOST_REQUIRE(service.startProducingStats(tableWindowTitle, observer()).empty());
      notifiedTables.clear();
    }
  }; // struct Fixture
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(TableServiceTest)

BOOST_AUTO_TEST_CASE(TableServiceTest_isPokerAppShouldSucceed) {
  Database db;
  const TableService ts {db};
  BOOST_REQUIRE(ts.isPokerApp("Winamax Poker"));
  BOOST_REQUIRE(ts.isPokerApp("PMU"));
  BOOST_REQUIRE(false == ts.isPokerApp("Some unimplemented site stem"));
}

BOOST_AUTO_TEST_CASE(TableServiceTest_startingWithoutHistoryShouldFail) {
  Database db;
  TableService ts {db};
  BOOST_TEST(!ts.startProducingStats(TESTVILLE_TITLE, [](TableStatistics&&) {}).empty());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_followingATableWithKnownStatsShouldNotifyAtOnce) {
  Fixture f;
  BOOST_REQUIRE(f.service.startProducingStats(TESTVILLE_TITLE, f.observer()).empty());
  BOOST_REQUIRE(1 == f.notifiedTables.size());
  BOOST_TEST("Testville" == f.notifiedTables.front());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_followingATableWithoutStatsShouldNotNotify) {
  Fixture f;
  BOOST_REQUIRE(f.service.startProducingStats("Winamax Nowhereville", f.observer()).empty());
  BOOST_TEST(f.notifiedTables.empty());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_savingTheHistoryOfAFollowedTableShouldNotify) {
  Fixture f;
  f.follow(TESTVILLE_TITLE);
  f.saved("20260101_Testville_real_holdem_no-limit.txt");
  BOOST_REQUIRE(1 == f.notifiedTables.size());
  BOOST_TEST("Testville" == f.notifiedTables.front());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_savingTheHistoryOfAnotherTableShouldNotNotify) {
  Fixture f;
  f.follow(TESTVILLE_TITLE);
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST(f.notifiedTables.empty());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_severalTablesShouldBeFollowedAtTheSameTime) {
  Fixture f;
  f.follow(TESTVILLE_TITLE);
  f.follow("Winamax Autreville");
  f.saved("20260101_Testville_real_holdem_no-limit.txt");
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST((std::vector<std::string> {"Testville", "Autreville"} == f.notifiedTables));
}

BOOST_AUTO_TEST_CASE(TableServiceTest_stoppedTablesShouldNotBeNotified) {
  Fixture f;
  f.follow(TESTVILLE_TITLE);
  f.follow("Winamax Autreville");
  f.service.stopProducingStatsForTable(TESTVILLE_TITLE);
  f.saved("20260101_Testville_real_holdem_no-limit.txt");
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST((std::vector<std::string> {"Autreville"} == f.notifiedTables));
  f.service.stopProducingStats();
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST(1 == f.notifiedTables.size());
}

BOOST_AUTO_TEST_SUITE_END()
