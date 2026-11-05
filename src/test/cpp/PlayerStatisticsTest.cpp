#include "TestInfrastructure.hpp"
#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "entities/Action.hpp" // ActionType, Street
#include "entities/Card.hpp"
#include "entities/Game.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp" // Time
#include "entities/Player.hpp"
#include "entities/Seat.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "statistics/PlayerStatistics.hpp"

namespace pt = phud::test;

static constexpr std::string_view HERO = "sabre_laser";

/**
 * @returns a Pmu site containing one hand where HERO raises preflop: a player can have the same
 * name on several sites. Built by hand as the Pmu hand builder is not implemented yet.
 */
[[nodiscard]] static std::unique_ptr<Site> newPmuSiteWithOneHeroRaise() {
  static constexpr std::string_view HAND_ID = "pmu-1";
  static constexpr std::string_view GAME_ID = "pmu-game";
  const Time date {{.strTime = "2026/01/03 20:00:00", .format = WINAMAX_HISTORY_TIME_FORMAT}};
  std::array<std::string, TableConstants::MAX_SEATS> seats {};
  seats[0] = HERO;
  seats[1] = "bob";
  const std::array<Card, TableConstants::MAX_CARDS> noCards {Card::none, Card::none, Card::none,
                                                             Card::none, Card::none};
  std::vector<std::unique_ptr<Action>> actions;
  actions.push_back(std::make_unique<Action>(Action::Params {.handId = HAND_ID,
                                                             .playerName = HERO,
                                                             .street = Street::preflop,
                                                             .type = ActionType::raise,
                                                             .actionIndex = 0,
                                                             .amount = 0.15}));
  Hand::Params handParams {.id = HAND_ID,
                           .gameType = GameType::cashGame,
                           .siteName = ProgramInfos::PMU_SITE_NAME,
                           .tableName = "PmuTable",
                           .buttonSeat = Seat::seatOne,
                           .maxSeats = Seat::seatTwo,
                           .level = 0,
                           .ante = 0,
                           .startDate = date,
                           .seatPlayers = seats,
                           .heroCards = noCards,
                           .boardCards = noCards,
                           .actions = std::move(actions)};
  auto pGame = std::make_unique<CashGame>(CashGame::Params {.id = GAME_ID,
                                                            .siteName = ProgramInfos::PMU_SITE_NAME,
                                                            .cashGameName = "PmuGame",
                                                            .variant = Variant::holdem,
                                                            .limit = Limit::noLimit,
                                                            .isRealMoney = true,
                                                            .nbMaxSeats = Seat::seatTwo,
                                                            .smallBlind = 0.02,
                                                            .bigBlind = 0.05,
                                                            .startDate = date});
  pGame->addHand(std::make_unique<Hand>(handParams));
  auto pSite = std::make_unique<Site>(ProgramInfos::PMU_SITE_NAME);
  auto pHero = std::make_unique<Player>(Player::Params {
      .name = HERO, .site = ProgramInfos::PMU_SITE_NAME, .comments = ""});
  pHero->setIsHero(true);
  pSite->addPlayer(std::move(pHero));
  pSite->addPlayer(std::make_unique<Player>(
      Player::Params {.name = "bob", .site = ProgramInfos::PMU_SITE_NAME, .comments = ""}));
  pSite->addGame(std::move(pGame));
  return pSite;
}

static void assertStats(const PlayerStatistics& stats, int nbHands, int vpip, int pfr) {
  BOOST_TEST(nbHands == stats.getNbHands());
  BOOST_TEST(vpip == stats.getVoluntaryPutMoneyInPot());
  BOOST_TEST(pfr == stats.getPreFlopRaise());
}

/**
 * Loads once Winamax/preflopStats, the hand-written hands detailed hand by hand with the rules in
 * TableStatisticsTest.cpp.
 *
 * Valeurs attendues, toutes tables confondues (Testville + Autreville = 7 + 4 = 11 mains) :
 *   sabre_laser  VPIP (3+3)/11 = 54.55  PFR (2+1)/11 = 27.27
 *   alice        VPIP (4+1)/11 = 45.45  PFR (3+1)/11 = 36.36
 *   bob          VPIP (2+0)/11 = 18.18  PFR (1+0)/11 = 9.09
 *
 * Les pourcentages sont tronques a l'affichage : 54.55 donne 54.
 */
struct PreflopStatsFixture {
  inline static std::unique_ptr<Site> pSite {nullptr};
  inline static std::unique_ptr<Database> pDatabase {nullptr};

  PreflopStatsFixture() {
    if (nullptr == pDatabase) {
      pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
      BOOST_REQUIRE(nullptr != pSite);
      pDatabase = std::make_unique<Database>();
      pDatabase->save(*pSite);
    }
  }
};

/* We need to be able to get all the stats for one given player */
BOOST_AUTO_TEST_SUITE(PlayerStatisticsTest)

BOOST_FIXTURE_TEST_CASE(PlayerStatisticsTest_statsShouldBeComputedByHandOverAllTables,
                        PreflopStatsFixture) {
  const auto pHero = pDatabase->readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, HERO);
  BOOST_REQUIRE(nullptr != pHero);
  BOOST_TEST(pHero->isHero());
  assertStats(*pHero, 11, 54, 27);
  const auto pAlice = pDatabase->readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, "alice");
  BOOST_REQUIRE(nullptr != pAlice);
  BOOST_TEST(!pAlice->isHero());
  assertStats(*pAlice, 11, 45, 36);
  const auto pBob = pDatabase->readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, "bob");
  BOOST_REQUIRE(nullptr != pBob);
  assertStats(*pBob, 11, 18, 9);
}

BOOST_FIXTURE_TEST_CASE(PlayerStatisticsTest_unknownPlayerShouldHaveNoStats, PreflopStatsFixture) {
  BOOST_TEST(nullptr ==
             pDatabase->readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, "nobody"));
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_statsShouldNotMixSites) {
  const auto pWinamax = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
  BOOST_REQUIRE(nullptr != pWinamax);
  const auto pPmu = newPmuSiteWithOneHeroRaise();
  Database db;
  db.save(*pWinamax);
  db.save(*pPmu);
  const auto pOnWinamax = db.readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, HERO);
  BOOST_REQUIRE(nullptr != pOnWinamax);
  assertStats(*pOnWinamax, 11, 54, 27);
  const auto pOnPmu = db.readPlayerStatistics(ProgramInfos::PMU_SITE_NAME, HERO);
  BOOST_REQUIRE(nullptr != pOnPmu);
  assertStats(*pOnPmu, 1, 100, 100);
}

BOOST_AUTO_TEST_CASE(PlayerStatisticsTest_computeVoluntaryPutMoneyInPotShouldSucceed) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/tc1591"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto pHero {db.readPlayerStatistics(ProgramInfos::WINAMAX_SITE_NAME, "tc1591")};
  BOOST_REQUIRE(nullptr != pHero);
  BOOST_TEST(ProgramInfos::WINAMAX_SITE_NAME == pHero->getSiteName());
  BOOST_REQUIRE(pHero->isHero());
  BOOST_REQUIRE(371 == pHero->getNbHands());
  BOOST_REQUIRE(42 == pHero->getVoluntaryPutMoneyInPot());
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
