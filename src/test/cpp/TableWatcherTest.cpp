#include "TestInfrastructure.hpp"
#include "gui/TableWatcher.hpp"

[[nodiscard]] static bool isPokerTable(std::string_view title) {
  constexpr std::string_view WINAMAX_TABLE_PATTERN { "Winamax" };
  // the window title should be something like 'Winamax someName someOptionalNumber'
  // e.g. 'Winamax Aalen 27', 'Winamax Athens'
  const auto nbSpaces { std::count(title.begin(), title.end(), ' ') };
  return
    title.starts_with(WINAMAX_TABLE_PATTERN) and
    (title.length() > WINAMAX_TABLE_PATTERN.length() + 3) and
    (' ' == title.at(WINAMAX_TABLE_PATTERN.length())) and
    (' ' != title.at(WINAMAX_TABLE_PATTERN.length() + 1)) and
    ((1 == nbSpaces) or (2 == nbSpaces));
}

BOOST_AUTO_TEST_SUITE(TableWatcherTest)

BOOST_AUTO_TEST_CASE(TableWatcherTest_shouldRecognizePokerTable) {
  BOOST_REQUIRE(true == isPokerTable("Winamax Nice 38"));
  BOOST_REQUIRE(true == isPokerTable("Winamax Nice"));
  BOOST_REQUIRE(false == isPokerTable("Winamax"));
  BOOST_REQUIRE(false == isPokerTable("Winamax Turbo Sat W Series Ticket 2 €(979135085)(#000)"));
}

BOOST_AUTO_TEST_SUITE_END()