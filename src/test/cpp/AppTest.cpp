#include "TestInfrastructure.hpp"
#include "mainLib/App.hpp" // AppInterface
#include "statistics/PlayerStatistics.hpp"

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(AppTest)

BOOST_AUTO_TEST_CASE(AppTest_enterAndExitShouldSucceed) {
  pt::TmpFile databaseFile;
  App mainProgram { databaseFile.string() };
  BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_SUITE_END()