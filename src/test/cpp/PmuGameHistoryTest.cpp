#include "TestInfrastructure.hpp" // LogDisabler, phud::test
// #include "entities/Game.hpp"
// #include "entities/Hand.hpp"
// #include "entities/Player.hpp"
// #include "entities/Site.hpp"
// #include "history/PmuGameHistory.hpp"

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(PmuGameHistoryTest)

BOOST_AUTO_TEST_CASE(PmuGameHistoryTest_loadingCashgameShouldSucceed) {
  // pas encore implémenté
  // const auto& file { pt::getFileFromTestResources("Pmu/sabre_laser/20210914/Reims.txt") };
  // const auto& pSite { PmuGameHistory::parseGameHistory(file) };
  // BOOST_REQUIRE(nullptr != pSite);
  // BOOST_REQUIRE(pSite->viewTournaments().empty());
  // BOOST_REQUIRE(!pSite->viewCashGames().empty());
  // const auto& cashgames { pSite->viewCashGames() };
  // BOOST_REQUIRE(1 == cashgames.size());
  // const auto& cg { *cashgames[0] };
  // BOOST_REQUIRE("Reims" == cg.getName());
  // BOOST_REQUIRE(true == cg.isRealMoney());
  // const auto& sd { cg.getStartDate() };
  // BOOST_REQUIRE("2021-09-14 18:34:28" == sd.toSqliteDate());
  // BOOST_REQUIRE("Pmu" == cg.getSiteName());
  // BOOST_REQUIRE("Reims" == cg.getId());
  // BOOST_REQUIRE(Variant::holdem == cg.getVariant());
  // BOOST_REQUIRE(Limit::noLimit == cg.getLimitType());
  // BOOST_REQUIRE(6 == cg.getMaxNbSeats());
  // BOOST_REQUIRE(0.02 == cg.getBigBlind());
  // BOOST_REQUIRE(31 == cg.viewHands().size());
}

BOOST_AUTO_TEST_SUITE_END()
