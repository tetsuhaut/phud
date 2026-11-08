#include "TestInfrastructure.hpp"
#include "gui/HudElement.hpp"
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface
#include "statistics/PlayerStatistics.hpp"
#include <set>

namespace piws = PlayerIndicatorWindow::surface;

[[nodiscard]] static const HudElement& get(std::string_view id) {
  const auto* const pElement = findHudElement(id);
  BOOST_REQUIRE(nullptr != pElement);
  return *pElement;
}

BOOST_AUTO_TEST_SUITE(HudElementTest)

BOOST_AUTO_TEST_CASE(HudElementTest_everyElementShouldBeDocumented) {
  std::ranges::for_each(getHudElements(), [](const HudElement& element) {
    BOOST_TEST(!element.id.empty());
    BOOST_TEST(!element.shortName.empty());
    BOOST_TEST(!element.name.empty());
    BOOST_TEST(!element.description.empty());
    BOOST_TEST(0 < element.widthInCells);
    BOOST_TEST(piws::nbColumns >= element.widthInCells);
  });
}

BOOST_AUTO_TEST_CASE(HudElementTest_idsShouldBeUnique) {
  std::set<std::string_view> ids;
  std::ranges::for_each(getHudElements(),
                        [&ids](const HudElement& element) { ids.insert(element.id); });
  BOOST_TEST(getHudElements().size() == ids.size());
}

BOOST_AUTO_TEST_CASE(HudElementTest_everyElementShouldFitInTheGridAtTheSameTime) {
  auto nbCells {0};
  std::ranges::for_each(getHudElements(), [&nbCells](const HudElement& element) {
    nbCells += element.widthInCells;
  });
  BOOST_TEST(piws::nbColumns * piws::nbRows >= nbCells);
}

BOOST_AUTO_TEST_CASE(HudElementTest_unknownIdShouldNotBeFound) {
  BOOST_TEST(nullptr == findHudElement("unknown"));
}

BOOST_AUTO_TEST_CASE(HudElementTest_statsNotComputedYetShouldBeListedWithoutData) {
  BOOST_TEST(!hasData(get("af")));
  BOOST_TEST(!hasData(get("flopCbet")));
  BOOST_TEST(hasData(get("vpip")));
}

BOOST_AUTO_TEST_CASE(HudElementTest_formatShouldDisplayThePlayerAndHisHands) {
  const PlayerStatistics ps {{.playerName = "alice",
                              .siteName = "site",
                              .isHero = false,
                              .nbHands = 1234,
                              .vpip = 24.6,
                              .pfr = 18}};
  BOOST_TEST("alice" == formatHudElement(get("playerName"), ps));
  BOOST_TEST("1234" == formatHudElement(get("nbHands"), ps));
  BOOST_TEST("18" == formatHudElement(get("pfr"), ps));
}

BOOST_AUTO_TEST_CASE(HudElementTest_formatShouldRoundThePercentages) {
  const PlayerStatistics ps {{.playerName = "alice",
                              .siteName = "site",
                              .isHero = false,
                              .nbHands = 100,
                              .vpip = 0,
                              .pfr = 0,
                              .nbFacedRaise = 10,
                              .threeBet = 7.6}};
  BOOST_TEST("8" == formatHudElement(get("threeBet"), ps));
}

BOOST_AUTO_TEST_CASE(HudElementTest_formatShouldDisplayADashWithoutData) {
  const PlayerStatistics ps {{.playerName = "alice",
                              .siteName = "site",
                              .isHero = false,
                              .nbHands = 100,
                              .vpip = 0,
                              .pfr = 0}};
  // not computed
  BOOST_TEST("-" == formatHudElement(get("af"), ps));
  // no opportunity: the player never faced a raise, nor a 3-bet
  BOOST_TEST("-" == formatHudElement(get("threeBet"), ps));
  BOOST_TEST("-" == formatHudElement(get("fourBet"), ps));
  // the opportunity exists, and it is really 0
  BOOST_TEST("0" == formatHudElement(get("vpip"), ps));
}

BOOST_AUTO_TEST_SUITE_END()
