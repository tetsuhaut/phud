#include "TestInfrastructure.hpp"
#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "gui/TableService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "statistics/PlayerStatistics.hpp" // required by TableStatistics
#include "statistics/TableStatistics.hpp"

namespace fs = std::filesystem;
namespace pt = phud::test;

namespace {
  constexpr std::string_view TESTVILLE_TITLE = "Winamax Testville";

  /**
   * A database containing the Winamax/preflopStats history, and a TableService using it.
   */
  struct [[nodiscard]] Fixture final {
    fs::path dir {pt::getDirFromTestResources("Winamax/preflopStats")};
    Database db {};
    TableService service {db};
    std::vector<std::string> notifiedTables {};

    Fixture() {
      db.save(*PokerSiteHistory::load(dir));
      service.setPokerSiteHistory(PokerSiteHistory::newInstance(dir));
    }

    [[nodiscard]] TableService::TableObserverCallback observer() {
      return [this](TableStatistics&& ts) {
        notifiedTables.emplace_back(ts.getTable());
      };
    }

    void saved(std::string_view fileName) {
      service.onHistoryFileSaved(dir / "history" / fileName);
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

BOOST_AUTO_TEST_CASE(TableServiceTest_savingTheHistoryOfAFollowedTableShouldNotify) {
  Fixture f;
  BOOST_REQUIRE(f.service.startProducingStats(TESTVILLE_TITLE, f.observer()).empty());
  f.saved("20260101_Testville_real_holdem_no-limit.txt");
  BOOST_REQUIRE(1 == f.notifiedTables.size());
  BOOST_TEST("Testville" == f.notifiedTables.front());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_savingTheHistoryOfAnotherTableShouldNotNotify) {
  Fixture f;
  BOOST_REQUIRE(f.service.startProducingStats(TESTVILLE_TITLE, f.observer()).empty());
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST(f.notifiedTables.empty());
}

BOOST_AUTO_TEST_CASE(TableServiceTest_severalTablesShouldBeFollowedAtTheSameTime) {
  Fixture f;
  BOOST_REQUIRE(f.service.startProducingStats(TESTVILLE_TITLE, f.observer()).empty());
  BOOST_REQUIRE(f.service.startProducingStats("Winamax Autreville", f.observer()).empty());
  f.saved("20260101_Testville_real_holdem_no-limit.txt");
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST((std::vector<std::string> {"Testville", "Autreville"} == f.notifiedTables));
}

BOOST_AUTO_TEST_CASE(TableServiceTest_stoppedTablesShouldNotBeNotified) {
  Fixture f;
  BOOST_REQUIRE(f.service.startProducingStats(TESTVILLE_TITLE, f.observer()).empty());
  BOOST_REQUIRE(f.service.startProducingStats("Winamax Autreville", f.observer()).empty());
  f.service.stopProducingStatsForTable(TESTVILLE_TITLE);
  f.saved("20260101_Testville_real_holdem_no-limit.txt");
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST((std::vector<std::string> {"Autreville"} == f.notifiedTables));
  f.service.stopProducingStats();
  f.saved("20260102_Autreville_real_holdem_no-limit.txt");
  BOOST_TEST(1 == f.notifiedTables.size());
}

BOOST_AUTO_TEST_SUITE_END()
