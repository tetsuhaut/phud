#include "TestInfrastructure.hpp"
#include "SabreLaserFixture.hpp" // Shared fixture for sabre_laser test data
#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "constants/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(TableStatisticsTest)

  BOOST_AUTO_TEST_CASE(TableStatisticsTest_readingStatisticsFromWinamaxTournamentShouldSucceed) {
    const auto& pSite { PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleTHisto")) };
    BOOST_REQUIRE(nullptr != pSite);
    Database db;
    db.save(*pSite);
    const auto& stats { db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Kill The Fish(152800689)#004") };
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
    // Use shared sabre_laser data loaded by fixture
    BOOST_REQUIRE(nullptr != pSabreLaserSite);
    Database db;
    db.save(*pSabreLaserSite);
    const auto& stats {
      db.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, "Double or Nothing(102140685)#0")
    };
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
