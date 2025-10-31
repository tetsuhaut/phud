#include "TestInfrastructure.hpp"
#include "TimeBomb.hpp" // std::chrono::*
#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "constants/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsProducer.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadSafeQueue.hpp"

namespace pt = phud::test;

static constexpr std::chrono::milliseconds COUNTDOWN_TO_EXPLOSION { 10000 };
static constexpr std::chrono::milliseconds SG_PERIOD { 1 };

BOOST_AUTO_TEST_SUITE(StatsGetterTest)

BOOST_AUTO_TEST_CASE(
  StatsGetterTest_parsingAnUpdatedHistoryShouldSucceed) { // TODO: dure 10 secondes ???
  const auto& pSite { PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/StatsGetterTest_parsingAnUpdatedHistoryShouldSucceed")) };
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  const auto& table { "Kill The Fish(152800689)#056" };
  /* create the timebomb after the database creation which can be slow */
  const StatsProducer producer { {.reloadPeriod = SG_PERIOD, .site = ProgramInfos::WINAMAX_SITE_NAME, .tableWindowName = table, .db = db} };
  ThreadSafeQueue<TableStatistics> statsQueue;
  producer.start(statsQueue);
  auto _ { TimeBomb::create(COUNTDOWN_TO_EXPLOSION, "StatsGetterTest_parsingAnUpdatedHistoryShouldSucceed") };
  TableStatistics stats;
  statsQueue.waitPop(stats);
  producer.stop();
  BOOST_REQUIRE(producer.isStopped());
  BOOST_REQUIRE(Seat::seatSix == stats.getMaxSeat());
  const auto& ps1 { stats.extractPlayerStatistics(Seat::seatOne) };
  /* sabre_laser was at the table, but did not play (just arrived) */
  BOOST_REQUIRE("lemonchelo69" == ps1->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == ps1->getSiteName());
  BOOST_REQUIRE(false == ps1->isHero());
  BOOST_REQUIRE_MESSAGE(0 == ps1->getVoluntaryPutMoneyInPot(),
                        ps1->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(0 == ps1->getPreFlopRaise());
  BOOST_REQUIRE(1 == ps1->getNbHands());
  const auto& ps2 { stats.extractPlayerStatistics(Seat::seatTwo) };
  BOOST_REQUIRE("Miraculum" == ps2->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == ps2->getSiteName());
  BOOST_REQUIRE(false == ps2->isHero());
  BOOST_REQUIRE(100 == ps2->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(100 == ps2->getPreFlopRaise());
  BOOST_REQUIRE(1 == ps2->getNbHands());
  const auto& ps3 { stats.extractPlayerStatistics(Seat::seatThree) };
  BOOST_REQUIRE("sabre_laser" == ps3->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == ps3->getSiteName());
  /* can't know yet since never got cards */
  BOOST_REQUIRE(false == ps3->isHero());
  BOOST_REQUIRE(0 == ps3->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(0 == ps3->getPreFlopRaise());
  BOOST_REQUIRE(1 == ps3->getNbHands());
  const auto& ps4 { stats.extractPlayerStatistics(Seat::seatFour) };
  BOOST_REQUIRE("anyamfia88" == ps4->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == ps4->getSiteName());
  BOOST_REQUIRE(false == ps4->isHero());
  BOOST_REQUIRE(0 == ps4->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(0 == ps4->getPreFlopRaise());
  BOOST_REQUIRE(1 == ps4->getNbHands());
  const auto& ps5 { stats.extractPlayerStatistics(Seat::seatFive) };
  BOOST_REQUIRE("Duflux" == ps5->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == ps5->getSiteName());
  BOOST_REQUIRE(false == ps5->isHero());
  BOOST_REQUIRE(0 == ps5->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(0 == ps5->getPreFlopRaise());
  BOOST_REQUIRE(1 == ps5->getNbHands());
  const auto& ps6 { stats.extractPlayerStatistics(Seat::seatSix) };
  BOOST_REQUIRE("aline1803" == ps6->getPlayerName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == ps6->getSiteName());
  BOOST_REQUIRE(false == ps6->isHero());
  BOOST_REQUIRE(0 == ps6->getVoluntaryPutMoneyInPot());
  BOOST_REQUIRE(0 == ps6->getPreFlopRaise());
  BOOST_REQUIRE(1 == ps6->getNbHands());
  BOOST_REQUIRE(nullptr == stats.extractPlayerStatistics(Seat::seatSeven));
  BOOST_REQUIRE(nullptr == stats.extractPlayerStatistics(Seat::seatEight));
  BOOST_REQUIRE(nullptr == stats.extractPlayerStatistics(Seat::seatNine));
  BOOST_REQUIRE(nullptr == stats.extractPlayerStatistics(Seat::seatTen));
}

BOOST_AUTO_TEST_SUITE_END()
