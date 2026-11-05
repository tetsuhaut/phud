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
 * Les pourcentages sont tronques a l'affichage : 57.14 donne 57 et 28.57 donne 28.
 */
BOOST_AUTO_TEST_CASE(TableStatisticsTest_statsShouldBeComputedByHandForTheGivenTableOnly) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/preflopStats"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto& testville = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Testville");
  BOOST_REQUIRE(Seat::seatThree == testville.getMaxSeat());
  assertStats(testville.m_tableStats[0].get(), "sabre_laser", 7, 42, 28);
  assertStats(testville.m_tableStats[1].get(), "alice", 7, 57, 42);
  assertStats(testville.m_tableStats[2].get(), "bob", 7, 28, 14);
  const auto& autreville = db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Autreville");
  assertStats(autreville.m_tableStats[0].get(), "sabre_laser", 4, 75, 25);
  assertStats(autreville.m_tableStats[1].get(), "alice", 4, 25, 25);
  assertStats(autreville.m_tableStats[2].get(), "bob", 4, 0, 0);
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
