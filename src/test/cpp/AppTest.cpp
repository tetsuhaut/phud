#include "TestInfrastructure.hpp"
#include "mainLib/App.hpp" // AppInterface

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(AppTest)

BOOST_AUTO_TEST_CASE(AppTest_enterAndExitShouldSucceed) {
  pt::TmpFile databaseFile;
  App mainProgram { databaseFile.string() };
  BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(AppTest_isPokerAppShouldSucceed) {
    pt::TmpFile databaseFile;
  App mainProgram { databaseFile.string() };
  BOOST_REQUIRE(mainProgram.isPokerApp("Winamax Poker"));
  BOOST_REQUIRE(mainProgram.isPokerApp("PMU"));  
  BOOST_REQUIRE(false == mainProgram.isPokerApp("Some unimplemented site stem"));
}

BOOST_AUTO_TEST_SUITE_END()