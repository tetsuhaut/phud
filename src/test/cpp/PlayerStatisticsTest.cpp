#include "TestInfrastructure.hpp"
#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "statistics/PlayerStatistics.hpp"

namespace pt = phud::test;

/* We need to be able to get all the stats for one given player */
BOOST_AUTO_TEST_SUITE(PlayerStatisticsTest)

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeVoluntaryPutMoneyInPotShouldSucceed) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/tc1591"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto pHero {db.readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, "tc1591")};
  BOOST_REQUIRE(nullptr != pHero);
  BOOST_TEST(ProgramInfos::WINAMAX_SITE_NAME == pHero->getSiteName());
  BOOST_REQUIRE(pHero->isHero());
  BOOST_REQUIRE(379 == pHero->getNbHands());
  BOOST_REQUIRE(45 == pHero->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(pHero->getVoluntaryPutMoneyInPot() > pHero->getPreFlopRaise());
  BOOST_REQUIRE(22 == pHero->getPreFlopRaise());
  /* TODO à finir
  BOOST_REQUIRE(0 < pHero->getAggressionFactor());
  BOOST_REQUIRE(4 >= pHero->getAggressionFactor());  */
  BOOST_REQUIRE_SMALL(pHero->getCallFlopCheckRaise(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getCallDonkBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getCallFlopContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getCallRiverContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getCallThreeBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getCallTurnContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getDonkBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFlopCheckRaise(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFlopContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToDonkBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToFlopCheckRaise(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToFlopContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToPreFlopRaise(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToRaiseAfterDonkBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToRaiseAfterFlopContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToRaiseAfterRiverContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToRaiseAfterTurnContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToRiverContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToThreeBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getFoldToTurnContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getLimpOrCallPreFlopRaise(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getRaiseAfterDonkBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getRaiseAfterFlopContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getRaiseAfterRiverContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getRaiseAfterTurnContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getRiverContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getPreflopThreeBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getTurnContinuationBet(), 1e-6);
  BOOST_REQUIRE_SMALL(pHero->getWentToShowDown(), 1e-6);
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computePreflopRaiseShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeLimpOrCallPreFlopRaiseShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToPreFlopRaiseShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeThreeBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToThreeBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallToThreeBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeAggressionFactorShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeWentToShowdownShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFlopContinuationBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToFlopContinuationBetShouldSucceed) { /*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallFlopContinuationBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeTurnContinuationBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToTurnContinuationBetShouldSucceed) { /*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallTurnContinuationBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(
    PlayerStatisticsTest_computeRaiseAfterTurnContinuationBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(
    PlayerStatisticsTest_computeFoldToRaiseAfterRiverContinuationBetShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeDonkBetShouldSucceed) { /*TODO*/ }
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToDonkBetShouldSucceed) { /*TODO*/ }
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallDonkBetShouldSucceed) { /*TODO*/ }
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeRaiseAfterDonkBetShouldSucceed) { /*TODO*/ }
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFlopCheckRaiseShouldSucceed) { /*TODO*/ }
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallToFlopCheckRaiseShouldSucceed) { /*TODO*/ }
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToFlopCheckRaiseShouldSucceed) { /*TODO*/ }

BOOST_AUTO_TEST_SUITE_END()
