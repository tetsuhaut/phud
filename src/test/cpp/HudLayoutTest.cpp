#include "TestInfrastructure.hpp"
#include "gui/HudElement.hpp"
#include "gui/HudLayout.hpp"
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface

namespace piws = PlayerIndicatorWindow::surface;

[[nodiscard]] static const HudElement& get(std::string_view id) {
  const auto* const pElement = findHudElement(id);
  BOOST_REQUIRE(nullptr != pElement);
  return *pElement;
}

BOOST_AUTO_TEST_SUITE(HudLayoutTest)

BOOST_AUTO_TEST_CASE(HudLayoutTest_defaultLayoutShouldDisplayNameHandsAndMainStats) {
  const auto layout = HudLayout::defaultLayout();
  BOOST_TEST(5 == layout.getItems().size());
  BOOST_TEST(layout.contains(get("playerName")));
  BOOST_TEST(layout.contains(get("nbHands")));
  BOOST_TEST(layout.contains(get("vpip")));
  BOOST_TEST(layout.contains(get("pfr")));
  BOOST_TEST(layout.contains(get("af")));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_serializeThenParseShouldGiveTheSameLayout) {
  auto layout = HudLayout::defaultLayout();
  BOOST_REQUIRE(layout.addElement(get("threeBet")));
  BOOST_REQUIRE(layout.moveElement(get("threeBet"), 7, 4));
  BOOST_TEST((layout == HudLayout::parse(layout.serialize())));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_everyElementShouldFitInSavedPreferences) {
  HudLayout layout;
  std::ranges::for_each(getHudElements(), [&layout](const HudElement& element) {
    BOOST_REQUIRE(layout.addElement(element));
  });
  BOOST_TEST((layout == HudLayout::parse(layout.serialize())));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_emptyLayoutShouldBeKept) {
  BOOST_TEST(HudLayout::parse("").getItems().empty());
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_parseShouldIgnoreWhatItCantRead) {
  const auto layout = HudLayout::parse(
      "vpip:1:0;unknown:0:1;pfr:x:0;pfr:0;;af:99:0;limp:0:99;threeBet:-1:0;squeeze:1:0");
  BOOST_TEST(1 == layout.getItems().size());
  BOOST_TEST(layout.contains(get("vpip")));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_parseShouldIgnoreDuplicates) {
  const auto layout = HudLayout::parse("vpip:0:0;vpip:3:3");
  BOOST_REQUIRE(1 == layout.getItems().size());
  BOOST_TEST(0 == layout.getItems().front().column);
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_addShouldUseTheFirstFreePlace) {
  HudLayout layout;
  BOOST_REQUIRE(layout.addElement(get("playerName")));
  BOOST_REQUIRE(layout.addElement(get("vpip")));
  const auto* const pItem = layout.find(get("vpip"));
  BOOST_REQUIRE(nullptr != pItem);
  BOOST_TEST(get("playerName").widthInCells == pItem->column);
  BOOST_TEST(0 == pItem->row);
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_addingTwiceShouldBeRefused) {
  HudLayout layout;
  BOOST_TEST(layout.addElement(get("vpip")));
  BOOST_TEST(!layout.addElement(get("vpip")));
  BOOST_TEST(1 == layout.getItems().size());
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeRefusedOverAnotherElement) {
  auto layout = HudLayout::parse("playerName:0:0;vpip:6:0");
  // the name covers the columns from 0 to 4
  BOOST_TEST(!layout.moveElement(get("vpip"), 4, 0));
  BOOST_TEST(6 == layout.find(get("vpip"))->column);
  BOOST_TEST(layout.moveElement(get("vpip"), 5, 0));
  BOOST_TEST(5 == layout.find(get("vpip"))->column);
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_wideElementShouldNotOverlapByItsRightSide) {
  auto layout = HudLayout::parse("vpip:4:0;playerName:0:1");
  BOOST_TEST(!layout.moveElement(get("playerName"), 0, 0));
  BOOST_TEST(layout.moveElement(get("playerName"), 5, 0));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeAllowedOverItself) {
  auto layout = HudLayout::parse("playerName:0:0");
  BOOST_TEST(layout.moveElement(get("playerName"), 2, 0));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeRefusedOutOfTheGrid) {
  auto layout = HudLayout::parse("playerName:0:0");
  BOOST_TEST(!layout.moveElement(get("playerName"), -1, 0));
  BOOST_TEST(!layout.moveElement(get("playerName"), 0, -1));
  BOOST_TEST(!layout.moveElement(get("playerName"), 0, piws::nbRows));
  BOOST_TEST(!layout.moveElement(get("playerName"), piws::nbColumns - 4, 0));
  BOOST_TEST(layout.moveElement(get("playerName"), piws::nbColumns - 5, piws::nbRows - 1));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeRefusedForAnElementNotDisplayed) {
  HudLayout layout;
  BOOST_TEST(!layout.moveElement(get("vpip"), 0, 0));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_elementAtShouldFindEveryCellOfAnElement) {
  const auto layout = HudLayout::parse("nbHands:3:2");
  BOOST_TEST(nullptr == layout.elementAt(2, 2));
  BOOST_TEST(&get("nbHands") == layout.elementAt(3, 2));
  BOOST_TEST(&get("nbHands") == layout.elementAt(4, 2));
  BOOST_TEST(nullptr == layout.elementAt(5, 2));
  BOOST_TEST(nullptr == layout.elementAt(3, 3));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_removeShouldFreeThePlace) {
  auto layout = HudLayout::parse("vpip:0:0");
  layout.removeElement(get("vpip"));
  BOOST_TEST(layout.getItems().empty());
  BOOST_TEST(nullptr == layout.elementAt(0, 0));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_sizeShouldBeTheBottomRightCorner) {
  const auto layout = HudLayout::parse("playerName:2:3");
  BOOST_TEST(7 * piws::cellWidth == layout.getWidth());
  BOOST_TEST(4 * piws::cellHeight == layout.getHeight());
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_emptyLayoutShouldKeepOneCell) {
  const HudLayout layout;
  BOOST_TEST(piws::cellWidth == layout.getWidth());
  BOOST_TEST(piws::cellHeight == layout.getHeight());
}

BOOST_AUTO_TEST_SUITE_END()
