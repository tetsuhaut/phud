#include "TestInfrastructure.hpp"
#include "db/Database.hpp"
#include "entities/Game.hpp"
#include "entities/Player.hpp"
#include "entities/Seat.hpp"
#include "entities/Site.hpp"
#include "filesystem/FileUtils.hpp" // phud::filesystem
#include "history/PokerSiteHistory.hpp"
#include "constants/ProgramInfos.hpp"
#include <gsl/gsl> // gsl::finally
#include <sqlite3.h>

namespace fs = std::filesystem;
namespace pt = phud::test;
namespace pf = phud::filesystem;

/**
 * Reads directly the database file: Database has no need to expose this.
 */
[[nodiscard]] static int countRows(const fs::path& dbFile, std::string_view table) {
  sqlite3* pDb {nullptr};
  BOOST_REQUIRE(SQLITE_OK == sqlite3_open(dbFile.string().c_str(), &pDb));
  const auto _ {gsl::finally([pDb] { sqlite3_close(pDb); })};
  const auto sql = std::format("SELECT COUNT(*) FROM {};", table);
  sqlite3_stmt* pStatement {nullptr};
  BOOST_REQUIRE(SQLITE_OK == sqlite3_prepare_v2(pDb, sql.c_str(), -1, &pStatement, nullptr));
  const auto __ {gsl::finally([pStatement] { sqlite3_finalize(pStatement); })};
  BOOST_REQUIRE(SQLITE_ROW == sqlite3_step(pStatement));
  return sqlite3_column_int(pStatement, 0);
}

static void assertPlayersAreOk(const Site& site) {
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
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleCGHisto"));
  BOOST_REQUIRE(nullptr != pSite);
  assertPlayersAreOk(*pSite);
  BOOST_REQUIRE(5 == pSite->viewCashGames().front()->viewHands("tc1591").size());
  Database db;
  db.save(*pSite->viewCashGames().front());
}

BOOST_AUTO_TEST_CASE(DatabaseTest_savingSimpleWinamaxCashGameAsynchShouldSucceed) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleCGHisto"));
  BOOST_REQUIRE(nullptr != pSite);
  assertPlayersAreOk(*pSite);
  BOOST_REQUIRE(5 == pSite->viewCashGames().front()->viewHands("tc1591").size());
  Database db;
  db.save(*pSite);
}

/* the whole history file is saved again each time a new hand is written in it */
BOOST_AUTO_TEST_CASE(DatabaseTest_savingTheSameHistoryTwiceShouldNotDuplicateActions) {
  const fs::path dbFile {"DatabaseTest_savingTwice.db"};
  const auto _ {gsl::finally([&] { fs::remove(dbFile); })};
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleCGHisto"));
  BOOST_REQUIRE(nullptr != pSite);
  Database db {dbFile.string()};
  db.save(*pSite);
  const auto nbActions = countRows(dbFile, "Action");
  BOOST_REQUIRE(0 < nbActions);
  db.save(*pSite);
  BOOST_TEST(nbActions == countRows(dbFile, "Action"));
}

BOOST_AUTO_TEST_CASE(DatabaseTest_shouldGetCorrectTableMaxSeat) {
  const auto pSite = PokerSiteHistory::load(pt::getDirFromTestResources("Winamax/simpleTHisto"));
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
  const auto dbFilePath = dbFile.path();
  Database namedDb {dbFile.string()};
  BOOST_REQUIRE(pf::isFile(dbFilePath));
  BOOST_REQUIRE(pf::isFile(fs::path(namedDb.getDbName())));
  BOOST_REQUIRE(dbFile.string() == namedDb.getDbName());
}

BOOST_AUTO_TEST_CASE(DatabaseTest_createNamedDatabaseWhenFileDoesNotExistsCreatesFile) {
  const fs::path dbFile {"someName"};
  const auto _ {gsl::finally([&] {
    if (pf::isFile(dbFile)) {
      fs::remove(dbFile);
    }
  })};

  if (pf::isFile(dbFile)) {
    fs::remove(dbFile);
  }

  BOOST_REQUIRE(!pf::isFile(dbFile));
  Database namedDb {dbFile.string()};
  BOOST_REQUIRE(pf::isFile(fs::path(namedDb.getDbName())));
  BOOST_REQUIRE(dbFile.string() == namedDb.getDbName());
}

BOOST_AUTO_TEST_SUITE_END()
