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

BOOST_AUTO_TEST_CASE(HudLayoutTest_savedFormatShouldBeIdColumnRow) {
  const auto layout = HudLayout::parse("vpip:3:1");
  BOOST_REQUIRE(1 == layout.getItems().size());
  BOOST_TEST((CellPosition {.row = 1, .column = 3} == layout.getItems().front().position));
  BOOST_TEST("vpip:3:1" == layout.serialize());
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_serializeThenParseShouldGiveTheSameLayout) {
  auto layout = HudLayout::defaultLayout();
  BOOST_REQUIRE(layout.addElement(get("threeBet")));
  BOOST_REQUIRE(layout.moveElement(get("threeBet"), {.row = 4, .column = 7}));
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
  BOOST_TEST((CellPosition {.row = 0, .column = 0} == layout.getItems().front().position));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_addShouldUseTheFirstFreePlace) {
  HudLayout layout;
  BOOST_REQUIRE(layout.addElement(get("playerName")));
  BOOST_REQUIRE(layout.addElement(get("vpip")));
  const auto* const pItem = layout.find(get("vpip"));
  BOOST_REQUIRE(nullptr != pItem);
  BOOST_TEST((CellPosition {.row = 0, .column = get("playerName").widthInCells} ==
              pItem->position));
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
  BOOST_TEST(!layout.moveElement(get("vpip"), {.row = 0, .column = 4}));
  BOOST_TEST(6 == layout.find(get("vpip"))->position.column);
  BOOST_TEST(layout.moveElement(get("vpip"), {.row = 0, .column = 5}));
  BOOST_TEST(5 == layout.find(get("vpip"))->position.column);
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_wideElementShouldNotOverlapByItsRightSide) {
  auto layout = HudLayout::parse("vpip:4:0;playerName:0:1");
  BOOST_TEST(!layout.moveElement(get("playerName"), {.row = 0, .column = 0}));
  BOOST_TEST(layout.moveElement(get("playerName"), {.row = 0, .column = 5}));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeAllowedOverItself) {
  auto layout = HudLayout::parse("playerName:0:0");
  BOOST_TEST(layout.moveElement(get("playerName"), {.row = 0, .column = 2}));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeRefusedOutOfTheGrid) {
  auto layout = HudLayout::parse("playerName:0:0");
  const auto& name = get("playerName");
  BOOST_TEST(!layout.moveElement(name, {.row = 0, .column = -1}));
  BOOST_TEST(!layout.moveElement(name, {.row = -1, .column = 0}));
  BOOST_TEST(!layout.moveElement(name, {.row = piws::nbRows, .column = 0}));
  BOOST_TEST(!layout.moveElement(name, {.row = 0, .column = piws::nbColumns - 4}));
  BOOST_TEST(layout.moveElement(name, {.row = piws::nbRows - 1, .column = piws::nbColumns - 5}));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_moveShouldBeRefusedForAnElementNotDisplayed) {
  HudLayout layout;
  BOOST_TEST(!layout.moveElement(get("vpip"), {.row = 0, .column = 0}));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_elementAtShouldFindEveryCellOfAnElement) {
  // two cells wide, at row 2, columns 3 and 4
  const auto layout = HudLayout::parse("nbHands:3:2");
  BOOST_TEST(nullptr == layout.elementAt({.row = 2, .column = 2}));
  BOOST_TEST(&get("nbHands") == layout.elementAt({.row = 2, .column = 3}));
  BOOST_TEST(&get("nbHands") == layout.elementAt({.row = 2, .column = 4}));
  BOOST_TEST(nullptr == layout.elementAt({.row = 2, .column = 5}));
  BOOST_TEST(nullptr == layout.elementAt({.row = 3, .column = 3}));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_removeShouldFreeThePlace) {
  auto layout = HudLayout::parse("vpip:0:0");
  layout.removeElement(get("vpip"));
  BOOST_TEST(layout.getItems().empty());
  BOOST_TEST(nullptr == layout.elementAt({.row = 0, .column = 0}));
}

BOOST_AUTO_TEST_CASE(HudLayoutTest_sizeShouldBeTheBottomRightCorner) {
  // five cells wide, at row 3, columns 2 to 6
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
