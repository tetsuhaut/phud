#include "TestInfrastructure.hpp"
#include "containers/algorithms.hpp"
#include "db/Database.hpp"
#include "db/sqliteQueries.hpp"
#include "entities/Game.hpp"
#include "entities/Hand.hpp"
#include "entities/Player.hpp"
#include "entities/Site.hpp"
#include "history/PokerSiteHistory.hpp"
#include "mainLib/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"

#include <gsl/gsl> // gsl::finally

namespace fs = std::filesystem;
namespace pa = phud::algorithms;
namespace pt = phud::test;
namespace pf = phud::filesystem;

static inline void assertPlayersAreOk(const Site& site) {
  BOOST_REQUIRE(21 == site.viewPlayers().size());
  BOOST_REQUIRE(nullptr != site.viewPlayer("Amntfs"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("tc1591"));
  BOOST_REQUIRE(site.viewPlayer("tc1591")->isHero());
  BOOST_REQUIRE(nullptr != site.viewPlayer("KT-Laplume74"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("martinh06"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("Akinos"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("JOOL81"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("Merkaba1111"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("daronwina"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("LeCavSeRebif"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("boa5ter"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("sicktricks"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("bebediego"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("Baroto"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("shorty31"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("gila90"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("acid rodge"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("CtD Jeyje"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("juju-63"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("mentalist61"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("MidnightSR"));
  BOOST_REQUIRE(nullptr != site.viewPlayer("nOnO_72"));
}

BOOST_AUTO_TEST_SUITE(DatabaseTest)

BOOST_AUTO_TEST_CASE(DatabaseTest_savingSimpleWinamaxCashGameShouldSucceed) {
  const auto& pSite { PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleCGHisto")) };
  BOOST_REQUIRE(nullptr != pSite);
  assertPlayersAreOk(*pSite);
  BOOST_REQUIRE(5 == pSite->viewCashGames().front()->viewHands("tc1591").size());
  Database db;
  db.save(*pSite->viewCashGames().front());
}

BOOST_AUTO_TEST_CASE(DatabaseTest_savingSimpleWinamaxCashGameAsynchShouldSucceed) {
  const auto& pSite { PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleCGHisto")) };
  BOOST_REQUIRE(nullptr != pSite);
  assertPlayersAreOk(*pSite);
  BOOST_REQUIRE(5 == pSite->viewCashGames().front()->viewHands("tc1591").size());
  Database db;
  db.save(*pSite);
}

BOOST_AUTO_TEST_CASE(DatabaseTest_shouldGetCorrectTableMaxSeat) {
  const auto& pSite { PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleTHisto")) };
  BOOST_REQUIRE(nullptr != pSite);
  Database db;
  db.save(*pSite);
  BOOST_REQUIRE(Seat::seatSix == db.getTableMaxSeat(ProgramInfos::WINAMAX_SITE_NAME,
                "Kill The Fish(152800689)#004"));
}

BOOST_AUTO_TEST_CASE(DatabaseTest_creatingInMemoryDatabaseShouldNotCreateFile) {
  Database inMemoryDb;
  BOOST_REQUIRE(!pf::isFile(fs::path(inMemoryDb.getDbName())));
  BOOST_REQUIRE(inMemoryDb.isInMemory());
}

BOOST_AUTO_TEST_CASE(DatabaseTest_createNamedDatabaseWhenFileAlreadyExistsShouldReuseDatabase) {
  pt::TmpFile dbFile;
  const auto& dbFilePath { dbFile.path() };
  Database namedDb { dbFile.string() };
  BOOST_REQUIRE(pf::isFile(dbFilePath));
  BOOST_REQUIRE(pf::isFile(fs::path(namedDb.getDbName())));
  BOOST_REQUIRE(dbFile.string() == namedDb.getDbName());
}

BOOST_AUTO_TEST_CASE(DatabaseTest_createNamedDatabaseWhenFileDoesNotExistsCreatesFile) {
  const fs::path dbFile { "someName" };
  auto _ { gsl::finally([&]{ if (pf::isFile(dbFile)) { std::filesystem::remove(dbFile); } }) };

  if (pf::isFile(dbFile)) { std::filesystem::remove(dbFile); }

  BOOST_REQUIRE(!pf::isFile(dbFile));
  Database namedDb { dbFile.string() };
  BOOST_REQUIRE(pf::isFile(fs::path(namedDb.getDbName())));
  BOOST_REQUIRE(dbFile.string() == namedDb.getDbName());
}

BOOST_AUTO_TEST_SUITE_END()