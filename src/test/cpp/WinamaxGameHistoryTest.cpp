#include "TestInfrastructure.hpp" // LogDisabler, phud::test
#include "entities/Game.hpp"
#include "entities/Hand.hpp"
#include "entities/Seat.hpp"
#include "entities/Site.hpp"
#include "history/WinamaxGameHistory.hpp"
#include "constants/ProgramInfos.hpp"

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(WinamaxGameHistoryTest)

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_doNotLoadHistoryWithExclamationPoint) {
  /* as these files are duplicates of the same without the exclamation point...*/
  pt::LogDisabler dummy;
  /* Winamax history files are encoded using utf-8. On Windows, file names are encoded using utf-16.
   The build system (i.e. CMake) must ensure that the compiler encodes all the strings in utf-8. */
  const auto& file {
    pt::getFileFromTestResources(
      u8"Winamax/sabre_laser/history/20150630_Super Freeroll Stade 1 - Déglingos !(123322389)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  BOOST_REQUIRE(pSite->viewTournaments().empty());
  BOOST_REQUIRE(pSite->viewPlayers().empty());
}

/* test the Hand parsing */

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingDoubleOrNothingTournamentShouldSucceed) {
  const auto& file { pt::getFileFromTestResources("Winamax/sabre_laser/history/20141116_Double or "
                     "Nothing(100679030)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  const auto& tournaments { pSite->viewTournaments() };
  BOOST_REQUIRE(1 == tournaments.size());
  const auto& t { *tournaments[0] };
  BOOST_REQUIRE("Double or Nothing(100679030)" == t.getName());
  BOOST_REQUIRE(true == t.isRealMoney());
  BOOST_REQUIRE("2014-11-16 20:50:22" == t.getStartDate().toSqliteDate());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == t.getSiteName());
  BOOST_REQUIRE("20141116_Double or Nothing(100679030)_real_holdem_no-limit" == t.getId());
  BOOST_REQUIRE(Variant::holdem == t.getVariant());
  BOOST_REQUIRE(Limit::noLimit == t.getLimitType());
  BOOST_REQUIRE(Seat::seatTen == t.getMaxNbSeats());
  BOOST_REQUIRE(2 == t.getBuyIn());
  BOOST_REQUIRE(22 == t.viewHands().size());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingFreerollTournamentShouldSucceed) {
  const auto& file { pt::getFileFromTestResources("Winamax/sabre_laser/history/20141119_Freeroll"
                     "(99427750)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingTournamentShouldSucceed) {
  const auto& file { pt::getFileFromTestResources("Winamax/sabre_laser/history/20150718_Hold'em("
                     "123703551)_play_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  const auto& tournaments { pSite->viewTournaments() };
  BOOST_REQUIRE(1 == tournaments.size());
  const auto& t { *tournaments[0] };
  BOOST_REQUIRE("Hold-em(123703551)_play_holdem_no-limit" == t.getName());
  BOOST_REQUIRE(false == t.isRealMoney());
  BOOST_REQUIRE("2015-07-18 14:03:09" == t.getStartDate().toSqliteDate());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == t.getSiteName());
  BOOST_REQUIRE("20150718_Hold-em(123703551)_play_holdem_no-limit" == t.getId());
  BOOST_REQUIRE(Variant::holdem == t.getVariant());
  BOOST_REQUIRE(Limit::noLimit == t.getLimitType());
  BOOST_REQUIRE(Seat::seatSix == t.getMaxNbSeats());
  BOOST_REQUIRE(5 == t.getBuyIn());
  BOOST_REQUIRE(51 == t.viewHands().size());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingCashGameShouldSucceed) {
  const auto& file { pt::getFileFromTestResources(
                       "Winamax/tc1591/history/20150309_Colorado 1_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(pSite->viewTournaments().empty());
  const auto& cashGames { pSite->viewCashGames() };
  BOOST_REQUIRE(1 == cashGames.size());
  const auto& cg { *cashGames[0] };
  BOOST_REQUIRE(0.01 == cg.getSmallBlind());
  BOOST_REQUIRE(0.02 == cg.getBigBlind());
  BOOST_REQUIRE("20150309_Colorado 1_real_holdem_no-limit" == cg.getId());
  BOOST_REQUIRE(Limit::noLimit == cg.getLimitType());
  BOOST_REQUIRE(Seat::seatSix == cg.getMaxNbSeats());
  BOOST_REQUIRE("Colorado 1" == cg.getName());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == cg.getSiteName());
  BOOST_REQUIRE("2015-03-09 22:19:08" == cg.getStartDate().toSqliteDate());
  BOOST_REQUIRE(Variant::holdem == cg.getVariant());
  BOOST_REQUIRE(cg.isRealMoney());
  BOOST_REQUIRE(5 == cg.viewHands().size());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingFreeMoneyCashGameShouldSucceed) {
  const auto& file { pt::getFileFromTestResources(
                       "Winamax/sabre_laser/history/20200404_Wichita 05_play_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  const auto& cg { *pSite->viewCashGames()[0] };
  BOOST_REQUIRE("20200404_Wichita 05_play_holdem_no-limit" == cg.getId());
  BOOST_REQUIRE("Wichita 05_play_holdem_no-limit" == cg.getName());
  BOOST_CHECK_MESSAGE("Wichita 05" == cg.viewHands()[0]->getTableName(),
                      "cg.viewHands()[0]->getTableName()=" + cg.viewHands()[0]->getTableName());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingDeglingosSuperFreerollShouldSucceed) {
  const auto& file { pt::getFileFromTestResources(
                       u8"Winamax/sabre_laser/history/20150630_Super Freeroll Stade 1 - Déglingos _(123322389)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  BOOST_REQUIRE(1 == pSite->viewTournaments().size());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_loadingOmaha5CashGameShouldSucceed) {
  const auto& file { pt::getFileFromTestResources(
                       "Winamax/omaha5CGHisto/history/20180304_Ferrare 04_real_omaha5_pot-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(1 == pSite->viewCashGames().size());
  BOOST_REQUIRE(pSite->viewTournaments().empty());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_mergingSitesShouldSucceed) {
  const auto& file1 { pt::getFileFromTestResources(
                        "Winamax/tc1591/history/20150309_Colorado 1_real_holdem_no-limit.txt") };
  const auto& pSiteToBeMerged { WinamaxGameHistory::parseGameHistory(file1) };
  BOOST_REQUIRE(21 == pSiteToBeMerged->viewPlayers().size());
  BOOST_REQUIRE(pt::isSet(pSiteToBeMerged->viewPlayers()));
  BOOST_REQUIRE(nullptr != pSiteToBeMerged->viewPlayer("tc1591"));
  BOOST_REQUIRE(5 == pSiteToBeMerged->viewCashGames()[0]->viewHands("tc1591").size());
  const auto& file2 { pt::getFileFromTestResources(
                        "Winamax/tc1591/history/20150309_Colorado 2_real_holdem_no-limit.txt") };
  auto site { WinamaxGameHistory::parseGameHistory(file2) };
  BOOST_REQUIRE(40 == site->viewPlayers().size());
  BOOST_REQUIRE(pt::isSet(site->viewPlayers()));
  BOOST_REQUIRE(10 == site->viewCashGames()[0]->viewHands("tc1591").size());
  site->merge(*pSiteToBeMerged);
  BOOST_REQUIRE(55 == site->viewPlayers().size());
  BOOST_REQUIRE(2 == site->viewCashGames().size());
  BOOST_REQUIRE(10 == site->viewCashGames()[0]->viewHands("tc1591").size());
  BOOST_REQUIRE(5 == site->viewCashGames()[1]->viewHands("tc1591").size());
  BOOST_REQUIRE(pt::isSet(site->viewPlayers()));
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_playerWithNoActionHaveActionNone) {
  const auto& file { pt::getFileFromTestResources("Winamax/simpleCGHisto/history/20150309_Colorado 1_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(21 == pSite->viewPlayers().size());
  BOOST_REQUIRE(1 == pSite->viewCashGames()[0]->viewHands("Merkaba1111").size());
  BOOST_REQUIRE(pSite->viewCashGames()[0]->viewHands("Merkaba1111")[0]->isWinner("Merkaba1111"));
}

BOOST_AUTO_TEST_CASE(TournamentTest_loadingDoubleOrNothingWithOnlyFoldsShouldSucceed) {
  const auto& file{ phud::test::getFileFromTestResources("Winamax/hands/20141031_Double or Nothing(98932321)_real_holdem_no-limit.txt") };
  const auto& pSite{ WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == pSite->getName());
  BOOST_REQUIRE(10 == pSite->viewPlayers().size());
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("Bordel92"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("Illuminatiz"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("sabre_laser"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("el fouAArat"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("DEEPstef"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("crazypix"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("blackviz"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("romjeresci"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("Smaky98"));
  BOOST_REQUIRE(nullptr != pSite->viewPlayer("djembe man"));
  BOOST_REQUIRE(nullptr == pSite->viewPlayer("toto"));
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  const auto& tournaments{ pSite->viewTournaments() };
  BOOST_REQUIRE(1 == tournaments.size());
  const auto& t{ *tournaments[0] };
  BOOST_REQUIRE_MESSAGE("Double or Nothing(98932321)" == t.getName(),
                        "t.getName()='" << t.getName() << '\'');
  BOOST_REQUIRE(t.isRealMoney());
  BOOST_REQUIRE(Time({ .strTime = "2014/10/31 00:45:01", .format = WINAMAX_HISTORY_TIME_FORMAT }) ==
                t.getStartDate());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == t.getSiteName());
  BOOST_REQUIRE("20141031_Double or Nothing(98932321)_real_holdem_no-limit" == t.getId());
  BOOST_REQUIRE(Variant::holdem == t.getVariant());
  BOOST_REQUIRE(Limit::noLimit == t.getLimitType());
  BOOST_REQUIRE(Seat::seatTen == t.getMaxNbSeats());
  BOOST_REQUIRE(2.0 == t.getBuyIn());
  BOOST_REQUIRE(1 == t.viewHands().size());
}

BOOST_AUTO_TEST_SUITE_END()