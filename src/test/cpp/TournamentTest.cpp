#include "TestInfrastructure.hpp"
#include "history/WinamaxGameHistory.hpp"
#include "entities/Game.hpp" // Tournament, Limit, Time, Variant
#include "entities/Hand.hpp"
#include "entities/Player.hpp"
#include "entities/Site.hpp"

BOOST_AUTO_TEST_SUITE(TournamentTest)

BOOST_AUTO_TEST_CASE(TournamentTest_loadingDoubleOrNothingWithOnlyFoldsShouldSucceed) {
  const auto& file { phud::test::getFileFromTestResources("Winamax/hands/20141031_Double or Nothing(98932321)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE("Winamax" == pSite->getName());
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
  const auto& tournaments { pSite->viewTournaments() };
  BOOST_REQUIRE(1 == tournaments.size());
  const auto& t { *tournaments[0] };
  BOOST_REQUIRE_MESSAGE("Double or Nothing(98932321)" == t.getName(),
    "t.getName()='" << t.getName() << '\'');
  BOOST_REQUIRE(t.isRealMoney());
  BOOST_REQUIRE(Time({ .strTime = "2014/10/31 00:45:01", .format = WINAMAX_HISTORY_TIME_FORMAT }) == t.getStartDate());
  BOOST_REQUIRE("Winamax" == t.getSiteName());
  BOOST_REQUIRE("20141031_Double or Nothing(98932321)_real_holdem_no-limit" == t.getId());
  BOOST_REQUIRE(Variant::holdem == t.getVariant());
  BOOST_REQUIRE(Limit::noLimit == t.getLimitType());
  BOOST_REQUIRE(Seat::seatTen == t.getMaxNbSeats());
  BOOST_REQUIRE(2.0 == t.getBuyIn());
  BOOST_REQUIRE(1 == t.viewHands().size());
}

BOOST_AUTO_TEST_SUITE_END()