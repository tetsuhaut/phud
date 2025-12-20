#include "TestInfrastructure.hpp" // TmpFile, std::string_view, LogDisabler
#include "SabreLaserFixture.hpp"  // Shared fixture for sabre_laser test data
#include "entities/Action.hpp"    // ActionType, Street
#include "entities/Game.hpp"      // CashGame, Tournament
#include "entities/Hand.hpp"
#include "entities/Site.hpp"
#include "filesystem/FileUtils.hpp" // phud::filesystem
#include "history/WinamaxHistory.hpp" // PokerSiteHistory, fs::*, std::*, buildTournament, buildCashGame
#include <unordered_set>

namespace fs = std::filesystem;
namespace pf = phud::filesystem;
namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(WinamaxHistoryTest)

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_doNotLookAtSummaries) {
  pt::TmpDir root {"WinamaxHistoryTest_doNotLookAtSummaries"};
  pt::TmpDir dir {root / "history"}, _1 {root / "data/buddy"}, _2 {root / "data/players"};
  pt::TmpFile _3 {dir / "toto_summary.txt"};
  pt::TmpFile _4 {dir / "winamax_positioning_file.dat"};
  std::unique_ptr<Site> pSite;
  {
    pt::LogDisabler _5;
    pSite = PokerSiteHistory::load(root.path());
  }
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(pSite->viewPlayers().empty());
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  BOOST_REQUIRE(pSite->viewTournaments().empty());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_parsingBadEmptyFileShouldNotProduceGame) {
  pt::TmpDir root {"WinamaxHistoryTest_parsingBadEmptyFileShouldNotProduceGame"};
  pt::TmpDir dir {root / "history"}, _1 {root / "data/buddy"}, _2 {root / "data/players"};
  pt::TmpFile emptyFileWithBadName {dir / "toto.txt"};
  pt::TmpFile _3 {dir / "toto_summary.txt"};
  pt::TmpFile _4 {dir / "winamax_positioning_file.dat"};
  BOOST_REQUIRE(pf::isFile(emptyFileWithBadName.path()));
  std::unique_ptr<Site> pSite;
  {
    pt::LogDisabler _;
    pSite = PokerSiteHistory::load(root.path());
  }
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  BOOST_REQUIRE(pSite->viewTournaments().empty());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_parsingEmptyFileShouldNotProduceGame) {
  pt::TmpDir root {"WinamaxHistoryTest_parsingEmptyFileShouldNotProduceGame"};
  pt::TmpDir dir {root / "history"}, _1 {root / "data/buddy"}, _2 {root / "data/players"};
  pt::TmpFile emptyFile {dir / "20141215_Double or Nothing(000000000)_real_holdem_no-limit.txt"};
  pt::TmpFile _3 {dir / "toto_summary.txt"};
  pt::TmpFile _4 {dir / "winamax_positioning_file.dat"};
  emptyFile.print("toto");
  BOOST_REQUIRE(pf::isDir(dir.path()));
  BOOST_REQUIRE(pf::isDir(root.path()));
  BOOST_REQUIRE(pf::isDir(_1.path()));
  BOOST_REQUIRE(pf::isDir(_2.path()));
  BOOST_REQUIRE(pf::isFile(emptyFile.path()));
  std::unique_ptr<Site> pSite;
  {
    pt::LogDisabler _;
    pSite = PokerSiteHistory::load(root.path());
  }
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  BOOST_REQUIRE(pSite->viewTournaments().empty());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_parsingGoodCashGameFileShouldSucceed) {
  /* looking at src\test\resources\Winamax\simpleCGHisto\history\20150309_Colorado
   * 1_real_holdem_no-limit.txt */
  const auto dir = pt::getDirFromTestResources("Winamax/simpleCGHisto");
  const auto pSite {PokerSiteHistory::load(dir)};
  BOOST_REQUIRE(1 == pSite->viewCashGames().size());
  BOOST_REQUIRE(pSite->viewTournaments().empty());
  const auto games = pSite->viewCashGames();
  const auto& cashGame = *games[0];
  BOOST_REQUIRE(cashGame.isRealMoney());
  BOOST_REQUIRE("2015-03-09 22:19:08" == cashGame.getStartDate().toSqliteDate());
  BOOST_REQUIRE("20150309_Colorado 1_real_holdem_no-limit" == cashGame.getId());
  BOOST_REQUIRE("Colorado 1" == cashGame.getName());
  const auto players = pSite->viewPlayers();
  BOOST_REQUIRE(21 == players.size());
  std::vector<std::string> actualPlayerNames;
  actualPlayerNames.reserve(players.size());
  std::transform(players.cbegin(), players.cend(), std::back_inserter(actualPlayerNames),
                 [](const auto& p) { return p->getName(); });
  BOOST_REQUIRE(pt::isSet(actualPlayerNames));
  std::vector<std::string> expectedPlayerNames {
      "Akinos",       "Amntfs",      "Baroto",     "JOOL81",     "CtD Jeyje",   "KT-Laplume74",
      "LeCavSeRebif", "Merkaba1111", "MidnightSR", "acid rodge", "bebediego",   "boa5ter",
      "daronwina",    "gila90",      "juju-63",    "martinh06",  "mentalist61", "nOnO_72",
      "shorty31",     "sicktricks",  "tc1591"};
  std::sort(expectedPlayerNames.begin(), expectedPlayerNames.end());
  std::sort(actualPlayerNames.begin(), actualPlayerNames.end());
  BOOST_REQUIRE(std::equal(expectedPlayerNames.begin(), expectedPlayerNames.end(),
                           actualPlayerNames.begin()));
  const auto hands = cashGame.viewHands();
  BOOST_REQUIRE(5 == hands.size());
  /* each hand has 6 players */
  std::ranges::for_each(hands, [](const auto& hand) {
    const auto seats = hand->getSeats();
    const std::span firstSix(seats.begin(), seats.begin() + 6);
    const auto isEmpty = [](auto s) {
      return s.empty();
    };
    BOOST_REQUIRE(std::ranges::none_of(firstSix, isEmpty));
    const std::span lastFour(seats.begin() + 6, seats.end());
    BOOST_REQUIRE(std::ranges::all_of(lastFour, isEmpty));
  });
  const std::vector<std::string> sixPlayerNames {"Amntfs",    "tc1591", "KT-Laplume74",
                                                 "martinh06", "Akinos", "JOOL81"};
  const auto firstHandSeats = hands[0]->getSeats();
  BOOST_REQUIRE(std::equal(sixPlayerNames.begin(), sixPlayerNames.end(), firstHandSeats.begin()));
  BOOST_REQUIRE(16 == hands[0]->viewActions().size());
  BOOST_REQUIRE(6 == hands[1]->viewActions().size());
  /* check that the 5 first players folded preflop */
  const auto actions = hands[1]->viewActions();

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

  const auto firstFive = std::span(&actions[0], &actions[5]);

#ifdef __clang__
#  pragma clang diagnostic pop
#endif

  std::ranges::for_each(firstFive, [](const auto& action) {
    BOOST_REQUIRE(Street::preflop == action->getStreet());
    BOOST_REQUIRE(ActionType::fold == action->getType());
  });
  /* the winner of the hand did not do anything, check that a none Action exists */
  BOOST_REQUIRE(Street::preflop == hands[1]->viewActions()[5]->getStreet());
  BOOST_REQUIRE(ActionType::none == hands[1]->viewActions()[5]->getType());
  BOOST_REQUIRE(16 == hands[2]->viewActions().size());
  BOOST_REQUIRE(9 == hands[3]->viewActions().size());
  BOOST_REQUIRE(12 == hands[4]->viewActions().size());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_parsingGoodTournamentFileShouldSucceed) {
  const auto dir = pt::getDirFromTestResources("Winamax/simpleTHisto");
  const auto pSite {PokerSiteHistory::load(dir)};
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  const auto games = pSite->viewTournaments();
  BOOST_REQUIRE(1 == games.size());
  const auto& tournament = *games[0];
  const auto hands = tournament.viewHands();
  BOOST_REQUIRE(216 == hands.size());
  BOOST_REQUIRE("Kill The Fish(152800689)" == tournament.getName());
  BOOST_REQUIRE(tournament.isRealMoney());
  BOOST_REQUIRE("2016-03-31 18:53:49" == tournament.getStartDate().toSqliteDate());
  BOOST_REQUIRE("20160331_Kill The Fish(152800689)_real_holdem_no-limit" == tournament.getId());
  BOOST_REQUIRE(30 == pSite->viewPlayers().size());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_shouldDetectInvalidHistoryDirectory) {
  pt::LogDisabler dummy;
  BOOST_REQUIRE(false == PokerSiteHistory::isValidHistory(pt::getTestResourcesDir()));
  BOOST_REQUIRE(false == PokerSiteHistory::isValidHistory(fs::path("toto")));
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_shouldDetectValidHistoryDirectory) {
  auto dir {pt::getDirFromTestResources("Winamax/tc1591")};
  const auto histoDir = dir.append("history").lexically_normal();
  auto subdirs = pf::listSubDirs(histoDir);
  BOOST_REQUIRE(PokerSiteHistory::isValidHistory(pt::getDirFromTestResources("Winamax/tc1591")));
  BOOST_REQUIRE(
      PokerSiteHistory::isValidHistory(pt::getDirFromTestResources("Winamax/sabre_laser")));
}

BOOST_FIXTURE_TEST_CASE(WinamaxHistoryTest_shouldNotHaveDuplicatedPlayers, SabreLaserFixture) {
  // Use shared sabre_laser data loaded by fixture
  auto players {pSabreLaserSite->viewPlayers()};
  std::unordered_set<const Player*> uniquePlayers;
  std::ranges::for_each(players, [&uniquePlayers](auto p) { uniquePlayers.insert(p); });
  BOOST_REQUIRE(players.size() == uniquePlayers.size());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_shouldGuessPlayMoneyCashGameTableNameFrowWindowTitle) {
  const auto psh = WinamaxHistory();
  // title from Winamax in 2015
  const auto table {
      psh.getTableNameFromTableWindowTitle("Wichita 02 / 0,01-0,02 NL Holdem / Argent fictif")};
  BOOST_TEST("Wichita 02" == table);
  // title from Winamax in 2025
  const auto table1 {psh.getTableNameFromTableWindowTitle("Winamax Wichita 08")};
  BOOST_TEST("Wichita 08" == table1);
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_shouldGuessSitngoTableNameFrowWindowTitle) {
  const auto psh = WinamaxHistory();
  const auto table {psh.getTableNameFromTableWindowTitle(
      "Sit&Go Freeroll(538892397)#0 / Buy-in: 0 € / 40-80 (ante 10) NL Holdem")};
  BOOST_TEST("Sit&Go Freeroll(538892397)" == table);
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_getHistoryFile20220101FromTableWindowTitleShouldSucceed) {
  const auto psh = WinamaxHistory();
  const auto dir {pt::getDirFromTestResources("Winamax/sabre_laser")};
  const auto oP {psh.getHistoryFileFromTableWindowTitle(
      dir, "Wichita 05 / 0,01-0,02 NL Holdem / Argent fictif")};
  const auto p = oP.value();
  // only the latest is found
  BOOST_TEST("20200404_Wichita 05_play_holdem_no-limit.txt" == p.filename().string());
}

BOOST_AUTO_TEST_CASE(WinamaxHistoryTest_getHistoryFile20250924FromTableWindowTitleShouldSucceed) {
  const auto wh = WinamaxHistory();
  const auto dir = pt::getDirFromTestResources("Winamax/sabre_laser");
  const auto oP {wh.getHistoryFileFromTableWindowTitle(dir, "Wichita 09")};
  const auto file {oP.value()};
  // only the latest is found
  BOOST_TEST("20250924_Wichita 09_play_holdem_no-limit.txt" == file.filename().string());
}

BOOST_AUTO_TEST_SUITE_END()
