#include "TestInfrastructure.hpp"
#include "db/Database.hpp"
#include "entities/Action.hpp"
#include "entities/Game.hpp"
#include "entities/Hand.hpp"
#include "entities/Player.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "constants/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"

namespace pt = phud::test;

/* We need to be able to get all the stats for one given player */
BOOST_AUTO_TEST_SUITE(PlayerStatisticsTest)

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeVoluntaryPutMoneyInPotShouldSucceed) {
  const auto& pSite { PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/tc1591")) };
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto pHero { db.readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, "tc1591") };
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
  BOOST_REQUIRE(0 == pHero->getCallFlopCheckRaise());
  BOOST_REQUIRE(0 == pHero->getCallDonkBet());
  BOOST_REQUIRE(0 == pHero->getCallFlopContinuationBet());
  BOOST_REQUIRE(0 == pHero->getCallRiverContinuationBet());
  BOOST_REQUIRE(0 == pHero->getCallThreeBet());
  BOOST_REQUIRE(0 == pHero->getCallTurnContinuationBet());
  BOOST_REQUIRE(0 == pHero->getDonkBet());
  BOOST_REQUIRE(0 == pHero->getFlopCheckRaise());
  BOOST_REQUIRE(0 == pHero->getFlopContinuationBet());
  BOOST_REQUIRE(0 == pHero->getFoldToDonkBet());
  BOOST_REQUIRE(0 == pHero->getFoldToFlopCheckRaise());
  BOOST_REQUIRE(0 == pHero->getFoldToFlopContinuationBet());
  BOOST_REQUIRE(0 == pHero->getFoldToPreFlopRaise());
  BOOST_REQUIRE(0 == pHero->getFoldToRaiseAfterDonkBet());
  BOOST_REQUIRE(0 == pHero->getFoldToRaiseAfterFlopContinuationBet());
  BOOST_REQUIRE(0 == pHero->getFoldToRaiseAfterRiverContinuationBet());
  BOOST_REQUIRE(0 == pHero->getFoldToRaiseAfterTurnContinuationBet());
  BOOST_REQUIRE(0 == pHero->getFoldToRiverContinuationBet());
  BOOST_REQUIRE(0 == pHero->getFoldToThreeBet());
  BOOST_REQUIRE(0 == pHero->getFoldToTurnContinuationBet());
  BOOST_REQUIRE(0 == pHero->getLimpOrCallPreFlopRaise());
  BOOST_REQUIRE(0 == pHero->getRaiseAfterDonkBet());
  BOOST_REQUIRE(0 == pHero->getRaiseAfterFlopContinuationBet());
  BOOST_REQUIRE(0 == pHero->getRaiseAfterRiverContinuationBet());
  BOOST_REQUIRE(0 == pHero->getRaiseAfterTurnContinuationBet());
  BOOST_REQUIRE(0 == pHero->getRiverContinuationBet());
  BOOST_REQUIRE(0 == pHero->getPreflopThreeBet());
  BOOST_REQUIRE(0 == pHero->getTurnContinuationBet());
  BOOST_REQUIRE(0 == pHero->getWentToShowDown());
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computePreflopRaiseShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeLimpOrCallPreFlopRaiseShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToPreFlopRaiseShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeThreeBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToThreeBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallToThreeBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeAggressionFactorShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeWentToShowdownShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFlopContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToFlopContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallFlopContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeTurnContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToTurnContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallTurnContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(
  PlayerStatisticsTest_computeRaiseAfterTurnContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(
  PlayerStatisticsTest_computeFoldToRaiseAfterRiverContinuationBetShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeDonkBetShouldSucceed) {/*TODO*/
}
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToDonkBetShouldSucceed) {/*TODO*/
}
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallDonkBetShouldSucceed) {/*TODO*/
}
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeRaiseAfterDonkBetShouldSucceed) {/*TODO*/
}
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFlopCheckRaiseShouldSucceed) {/*TODO*/
}
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeCallToFlopCheckRaiseShouldSucceed) {/*TODO*/
}
BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeFoldToFlopCheckRaiseShouldSucceed) {/*TODO*/
}

BOOST_AUTO_TEST_SUITE_END()