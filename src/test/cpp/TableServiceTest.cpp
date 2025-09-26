#include "TestInfrastructure.hpp"
#include "db/Database.hpp"
#include "gui/TableService.hpp"

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(TableServiceTest)

BOOST_AUTO_TEST_CASE(TableServiceTest_isPokerAppShouldSucceed) {
  Database db;
  const TableService ts { db };
  BOOST_REQUIRE(ts.isPokerApp("Winamax Poker"));
  BOOST_REQUIRE(ts.isPokerApp("PMU"));
  BOOST_REQUIRE(false == ts.isPokerApp("Some unimplemented site stem"));
}

BOOST_AUTO_TEST_SUITE_END()
