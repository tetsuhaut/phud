#include "TestInfrastructure.hpp"
#include "SabreLaserFixture.hpp" // Shared fixture for sabre_laser test data
#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "constants/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"

namespace pt = phud::test;

/**
 * Shared fixture for simpleTHisto test data.
 * Loads once and reuses across tests for better performance.
 */
struct SimpleTHistoFixture {
  inline static std::unique_ptr<Site> pSite {nullptr};
  inline static std::unique_ptr<Database> pDatabase {nullptr};
  inline static bool isLoaded = false;

  SimpleTHistoFixture() {
    if (!isLoaded) {
      pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleTHisto"));
      pDatabase = std::make_unique<Database>();
      pDatabase->save(*pSite);
      isLoaded = true;
    }
  }
};

static void assertStats(const PlayerStatistics* pStats, std::string_view name, int nbHands, int vpip,
                        int pfr) {
  BOOST_REQUIRE(nullptr != pStats);
  BOOST_TEST(name == pStats->getPlayerName());
  BOOST_TEST(nbHands == pStats->getNbHands());
  BOOST_TEST(vpip == pStats->getVoluntaryPutMoneyInPot());
  BOOST_TEST(pfr == pStats->getPreFlopRaise());
}

BOOST_AUTO_TEST_SUITE(TableStatisticsTest)

/*
 * Winamax/preflopStats est un jeu de mains ecrit a la main : les valeurs attendues se calculent de
 * tete, independamment du code. Chaque joueur est assis a toutes les mains.
 * Les statistiques couvrent tout l'historique du joueur : Testville et Autreville donnent donc les
 * memes valeurs, celles des 11 mains.
 *
 * Regles (cf. doc/spec_stat.txt) :
 *   VPIP = mains avec call ou raise preflop / mains jouees
 *   PFR  = mains avec raise preflop / mains jouees
 *   Une main compte une seule fois, quel que soit le nombre d'actions du joueur.
 *   Les blindes postees et les pots ramasses ne comptent pas.
 *
 * Testville (7 mains)
 *   main  bouton       sabre_laser      alice            bob
 *   1     sabre_laser  raise, gagne     fold             fold
 *   2     alice        check            call             fold
 *   3     bob          fold             walk (gagne)     fold
 *   4     sabre_laser  raise, call      3-bet            fold
 *   5     alice        walk (gagne)     fold             fold
 *   6     bob          call, call       squeeze          raise, fold
 *   7     sabre_laser  fold             raise            call
 *
 *   sabre_laser  VPIP 3/7 = 42.86  PFR 2/7 = 28.57
 *   alice        VPIP 4/7 = 57.14  PFR 3/7 = 42.86
 *   bob          VPIP 2/7 = 28.57  PFR 1/7 = 14.29
 *
 * Autreville (4 mains)
 *   main  bouton       sabre_laser      alice            bob
 *   1     sabre_laser  raise, gagne     fold             fold
 *   2     alice        call             raise            fold
 *   3     bob          call             check            fold
 *   4     sabre_laser  fold             fold             walk (gagne)
 *
 *   sabre_laser  VPIP 3/4 = 75  PFR 1/4 = 25
 *   alice        VPIP 1/4 = 25  PFR 1/4 = 25
 *   bob          VPIP 0/4 = 0   PFR 0/4 = 0
 *
 * Toutes tables confondues (Testville + Autreville = 11 mains)
 *   sabre_laser  VPIP 6/11 = 54.55  PFR 3/11 = 27.27
 *   alice        VPIP 5/11 = 45.45  PFR 4/11 = 36.36
 *   bob          VPIP 2/11 = 18.18  PFR 1/11 = 9.09
 *
 * Face a une relance (Tn = main n de Testville, An = main n d'Autreville)
 *   sabre_laser  2 fois (T6, A2) : 2 calls
 *   alice        4 fois (T1, T4, T6, A1) : 2 folds (T1, A1), 2 3-bets (T4, T6)
 *   bob          4 fois (T1, T7, A1, A2) : 3 folds, 1 call (T7)
 * Face a un 3-bet
 *   sabre_laser  1 fois (T4) : 1 call
 *   alice        jamais
 *   bob          1 fois (T6) : 1 fold
 * Squeeze : seule alice en a l'occasion (T6), et le fait.
 * Limp : sabre_laser 1 fois (A3), alice 1 fois (T2), bob jamais.
 *
 * VPIP et PFR sont tronques a l'affichage : 54.55 donne 54 et 27.27 donne 27.
 */
BOOST_AUTO_TEST_CASE(TableStatisticsTest_statsShouldCoverTheWholeHistoryOfThePlayers) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto& testville = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Testville");
  BOOST_REQUIRE(Seat::seatThree == testville.getMaxSeat());
  assertStats(testville.m_tableStats[0].get(), "sabre_laser", 11, 54, 27);
  assertStats(testville.m_tableStats[1].get(), "alice", 11, 45, 36);
  assertStats(testville.m_tableStats[2].get(), "bob", 11, 18, 9);
  const auto& autreville = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Autreville");
  assertStats(autreville.m_tableStats[0].get(), "sabre_laser", 11, 54, 27);
  assertStats(autreville.m_tableStats[1].get(), "alice", 11, 45, 36);
  assertStats(autreville.m_tableStats[2].get(), "bob", 11, 18, 9);
}

BOOST_AUTO_TEST_CASE(TableStatisticsTest_preflopStatsShouldBeTheHandComputedValues) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto& stats = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Testville");
  const auto tolerance = boost::test_tools::tolerance(1e-6);

  const auto* pHero = stats.m_tableStats[0].get();
  BOOST_REQUIRE(nullptr != pHero);
  BOOST_TEST(100.0 / 11 == pHero->getLimp(), tolerance);
  BOOST_TEST(2 == pHero->getNbFacedRaise());
  BOOST_TEST(0.0 == pHero->getFoldToPreFlopRaise());
  BOOST_TEST(100.0 == pHero->getCallPreFlopRaise(), tolerance);
  BOOST_TEST(0.0 == pHero->getPreflopThreeBet());
  BOOST_TEST(1 == pHero->getNbFacedThreeBet());
  BOOST_TEST(0.0 == pHero->getFoldToThreeBet());
  BOOST_TEST(100.0 == pHero->getCallThreeBet(), tolerance);
  BOOST_TEST(0.0 == pHero->getFourBet());

  const auto* pAlice = stats.m_tableStats[1].get();
  BOOST_REQUIRE(nullptr != pAlice);
  BOOST_TEST(100.0 / 11 == pAlice->getLimp(), tolerance);
  BOOST_TEST(4 == pAlice->getNbFacedRaise());
  BOOST_TEST(50.0 == pAlice->getFoldToPreFlopRaise(), tolerance);
  BOOST_TEST(0.0 == pAlice->getCallPreFlopRaise());
  BOOST_TEST(50.0 == pAlice->getPreflopThreeBet(), tolerance);
  BOOST_TEST(100.0 == pAlice->getSqueeze(), tolerance);
  BOOST_TEST(0 == pAlice->getNbFacedThreeBet());

  const auto* pBob = stats.m_tableStats[2].get();
  BOOST_REQUIRE(nullptr != pBob);
  BOOST_TEST(0.0 == pBob->getLimp());
  BOOST_TEST(4 == pBob->getNbFacedRaise());
  BOOST_TEST(75.0 == pBob->getFoldToPreFlopRaise(), tolerance);
  BOOST_TEST(25.0 == pBob->getCallPreFlopRaise(), tolerance);
  BOOST_TEST(1 == pBob->getNbFacedThreeBet());
  BOOST_TEST(100.0 == pBob->getFoldToThreeBet(), tolerance);
}

/* sabre_laser joue aussi une main de tournoi (raise) et une main d'Escape (fold) */
BOOST_AUTO_TEST_CASE(TableStatisticsTest_statsShouldNotMixGameTypes) {
  const auto pCashGames =
      PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
  BOOST_REQUIRE(nullptr != pCashGames);
  const auto pOtherGames =
      PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStatsOtherGames"));
  BOOST_REQUIRE(nullptr != pOtherGames);
  Database db;
  db.save(*pCashGames);
  db.save(*pOtherGames);
  const auto& cashGame = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Testville");
  assertStats(cashGame.m_tableStats[0].get(), "sabre_laser", 11, 54, 27);
  const auto& tournament =
      db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Tournoi(123456)#001");
  assertStats(tournament.m_tableStats[0].get(), "sabre_laser", 1, 100, 100);
  const auto& fastFold = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Fastville");
  assertStats(fastFold.m_tableStats[0].get(), "sabre_laser", 1, 0, 0);
}

/* the whole history file is saved again each time a new hand is written in it */
BOOST_AUTO_TEST_CASE(TableStatisticsTest_savingTheSameHistoryTwiceShouldNotChangeTheStats) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  db.save(*pSite);
  const auto& stats = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Testville");
  assertStats(stats.m_tableStats[0].get(), "sabre_laser", 11, 54, 27);
  BOOST_TEST(2 == stats.m_tableStats[0]->getNbFacedRaise());
}

BOOST_FIXTURE_TEST_CASE(TableStatisticsTest_readingStatisticsFromWinamaxTournamentShouldSucceed,
                        SimpleTHistoFixture) {
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(nullptr != pDatabase);
  const auto& stats = pDatabase->readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME,
                                                     "Kill The Fish(152800689)#004");
  BOOST_REQUIRE(Seat::seatSix == stats.getMaxSeat());
  BOOST_REQUIRE("StopCallFish" == stats.m_tableStats[0]->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == stats.m_tableStats[0]->getSiteName());
  BOOST_REQUIRE("DelAmri" == stats.m_tableStats[1]->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == stats.m_tableStats[1]->getSiteName());
  BOOST_REQUIRE("Herlock33" == stats.m_tableStats[2]->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == stats.m_tableStats[2]->getSiteName());
  BOOST_REQUIRE(nullptr == stats.m_tableStats[3]);
  BOOST_REQUIRE("sabre_laser" == stats.m_tableStats[4]->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == stats.m_tableStats[4]->getSiteName());
  BOOST_REQUIRE(stats.m_tableStats[4]->isHero());
  BOOST_REQUIRE(Seat::seatFive == stats.getHeroSeat());
  BOOST_REQUIRE("Gengispain" == stats.m_tableStats[5]->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == stats.m_tableStats[5]->getSiteName());
  BOOST_REQUIRE(nullptr == stats.m_tableStats[6]);
  BOOST_REQUIRE(nullptr == stats.m_tableStats[7]);
  BOOST_REQUIRE(nullptr == stats.m_tableStats[8]);
  BOOST_REQUIRE(nullptr == stats.m_tableStats[9]);
}

BOOST_FIXTURE_TEST_CASE(TableStatisticsTest_readingTablePlayersShouldSucceed, SabreLaserFixture) {
  BOOST_REQUIRE(nullptr != pSabreLaserSite);
  BOOST_REQUIRE(nullptr != pDatabase);
  const auto& stats {pDatabase->readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME,
                                                    "Double or Nothing(102140685)#0")};
  BOOST_REQUIRE("secretstar62" == stats.m_tableStats[0]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[0]->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE("LECOLOMBIER7" == stats.m_tableStats[1]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[1]->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE("G_ZU" == stats.m_tableStats[2]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[2]->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE("sabre_laser" == stats.m_tableStats[3]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[3]->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(stats.m_tableStats[3]->isHero());
  BOOST_REQUIRE(Seat::seatFour == stats.getHeroSeat());
  BOOST_REQUIRE(stats.m_tableStats[3]->isHero());
  BOOST_REQUIRE("fanatic1980" == stats.m_tableStats[4]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[4]->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(nullptr == stats.m_tableStats[5]);
  BOOST_REQUIRE("jerem59133" == stats.m_tableStats[6]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[6]->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(nullptr == stats.m_tableStats[7]);
  BOOST_REQUIRE(nullptr == stats.m_tableStats[8]);
  BOOST_REQUIRE("sianae" == stats.m_tableStats[9]->getPlayerName());
  BOOST_REQUIRE(0 <= stats.m_tableStats[9]->getVoluntaryPutMoneyInPot());
}

BOOST_AUTO_TEST_SUITE_END()
