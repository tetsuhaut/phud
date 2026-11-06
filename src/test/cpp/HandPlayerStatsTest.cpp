#include "TestInfrastructure.hpp"
#include "entities/Game.hpp"
#include "entities/Hand.hpp"
#include "entities/Site.hpp"
#include "history/WinamaxGameHistory.hpp"
#include "statistics/HandPlayerStats.hpp"

namespace pt = phud::test;

[[nodiscard]] static const HandPlayerStats& find(const std::vector<HandPlayerStats>& stats,
                                                 std::string_view playerName) {
  const auto it = std::ranges::find_if(
      stats, [playerName](const auto& s) { return playerName == s.playerName; });
  BOOST_REQUIRE(stats.end() != it);
  return *it;
}

/**
 * @returns the stats of each hand of Winamax/preflopStats/history/<fileName>, in the file order.
 * The hands are detailed in TableStatisticsTest.cpp.
 */
[[nodiscard]] static std::vector<std::vector<HandPlayerStats>> readHands(std::string_view fileName) {
  const auto file = pt::getFileFromTestResources(
      std::format("Winamax/preflopStats/history/{}", fileName));
  const auto pSite = WinamaxGameHistory::parseGameHistory(file);
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(1 == pSite->viewCashGames().size());
  std::vector<std::vector<HandPlayerStats>> ret;
  std::ranges::transform(pSite->viewCashGames().front()->viewHands(), std::back_inserter(ret),
                         [](const auto* pHand) { return computeHandPlayerStats(*pHand); });
  return ret;
}

static constexpr std::string_view TESTVILLE = "20260101_Testville_real_holdem_no-limit.txt";
static constexpr std::string_view AUTREVILLE = "20260102_Autreville_real_holdem_no-limit.txt";

BOOST_AUTO_TEST_SUITE(HandPlayerStatsTest)

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_everySeatedPlayerShouldHaveStats) {
  const auto hands = readHands(TESTVILLE);
  BOOST_REQUIRE(7 == hands.size());
  std::ranges::for_each(hands, [](const auto& players) { BOOST_TEST(3 == players.size()); });
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_openRaiseShouldBeVpipAndPfrOnly) {
  const auto hands = readHands(TESTVILLE);
  const auto& hero = find(hands[0], "sabre_laser");
  BOOST_TEST(hero.vpip);
  BOOST_TEST(hero.pfr);
  BOOST_TEST(!hero.limp);
  BOOST_TEST(!hero.facedRaise);
  BOOST_TEST(!hero.threeBet);
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_foldingToARaiseShouldBeAFacedRaise) {
  const auto hands = readHands(TESTVILLE);
  const auto& alice = find(hands[0], "alice");
  BOOST_TEST(!alice.vpip);
  BOOST_TEST(alice.facedRaise);
  BOOST_TEST(alice.foldToRaise);
  BOOST_TEST(!alice.callRaise);
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_walkShouldBeNoAction) {
  const auto hands = readHands(TESTVILLE);
  const auto& hero = find(hands[4], "sabre_laser");
  BOOST_TEST(!hero.vpip);
  BOOST_TEST(!hero.pfr);
  BOOST_TEST(!hero.limp);
  BOOST_TEST(!hero.facedRaise);
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_callingTheBigBlindFirstShouldBeALimp) {
  const auto testville = readHands(TESTVILLE);
  BOOST_TEST(find(testville[1], "alice").limp);
  BOOST_TEST(!find(testville[1], "alice").pfr);
  const auto autreville = readHands(AUTREVILLE);
  BOOST_TEST(find(autreville[2], "sabre_laser").limp);
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_threeBetShouldBeSeenByBothPlayers) {
  const auto hands = readHands(TESTVILLE);
  const auto& alice = find(hands[3], "alice");
  BOOST_TEST(alice.facedRaise);
  BOOST_TEST(alice.threeBet);
  BOOST_TEST(!alice.couldSqueeze);
  const auto& hero = find(hands[3], "sabre_laser");
  BOOST_TEST(!hero.facedRaise);
  BOOST_TEST(hero.pfr);
  BOOST_TEST(hero.facedThreeBet);
  BOOST_TEST(hero.callThreeBet);
  BOOST_TEST(!hero.foldToThreeBet);
  BOOST_TEST(!hero.fourBet);
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_foldingAfterAThreeBetShouldNotBeAFacedRaise) {
  const auto hands = readHands(TESTVILLE);
  const auto& bob = find(hands[3], "bob");
  BOOST_TEST(!bob.facedRaise);
  BOOST_TEST(!bob.foldToRaise);
  BOOST_TEST(!bob.facedThreeBet);
}

BOOST_AUTO_TEST_CASE(HandPlayerStatsTest_raisingARaiseAndACallShouldBeASqueeze) {
  const auto hands = readHands(TESTVILLE);
  const auto& alice = find(hands[5], "alice");
  BOOST_TEST(alice.couldSqueeze);
  BOOST_TEST(alice.squeeze);
  BOOST_TEST(alice.threeBet);
  const auto& hero = find(hands[5], "sabre_laser");
  BOOST_TEST(hero.facedRaise);
  BOOST_TEST(hero.callRaise);
  BOOST_TEST(!hero.couldSqueeze);
  const auto& bob = find(hands[5], "bob");
  BOOST_TEST(bob.facedThreeBet);
  BOOST_TEST(bob.foldToThreeBet);
}

BOOST_AUTO_TEST_SUITE_END()
