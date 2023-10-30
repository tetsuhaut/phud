#include "TestInfrastructure.hpp"
#include "containers/algorithms.hpp"
#include "gui/Rectangle.hpp"
#include "gui/Position.hpp"

#include <set>

namespace pa = phud::algorithms;

BOOST_AUTO_TEST_SUITE(PositionTest)

BOOST_AUTO_TEST_CASE(PositionTest_buildPlayerIndicatorPositionShouldSucceed) {
  const auto& [x1, y1] { buildPlayerIndicatorPosition(Seat::seatOne, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
  const auto& [x2, y2] { buildPlayerIndicatorPosition(Seat::seatTwo, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
  const auto& [x3, y3] { buildPlayerIndicatorPosition(Seat::seatThree, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
  const auto& [x4, y4] { buildPlayerIndicatorPosition(Seat::seatFour, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
  const auto& [x5, y5] { buildPlayerIndicatorPosition(Seat::seatFive, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
  const auto& [x6, y6] { buildPlayerIndicatorPosition(Seat::seatSix, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
}

BOOST_AUTO_TEST_CASE(PositionTest_playerIndicatorsShouldBeLocatedInsideTheTableWindow) {
  phud::Rectangle tablePosition { .x = 50, .y = 50, .w = 300, .h = 200 };
  const auto seats = { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix };
  pa::forEach(seats, [&tablePosition](auto seat) {
    auto [x, y] { buildPlayerIndicatorPosition(seat, Seat::seatThree, Seat::seatSix, tablePosition) };
    BOOST_REQUIRE(tablePosition.x <= x and x <= (tablePosition.x + tablePosition.w));
    BOOST_REQUIRE(50 <= tablePosition.y and (tablePosition.y + tablePosition.h) <= 250);
  });
}

BOOST_AUTO_TEST_CASE(PositionTest_playerIndicatorsShouldHaveDistinctPositions) {
  phud::Rectangle tablePosition { .x = 50, .y = 50, .w = 300, .h = 200 };
  const auto seats = { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix };
  std::vector<std::pair<int, int>> positions;
  positions.reserve(seats.size());
  pa::forEach(seats, [&tablePosition, &positions](auto seat) {
    positions.push_back(buildPlayerIndicatorPosition(seat, Seat::seatThree, Seat::seatSix,
                        tablePosition));
  });
  std::set<std::pair<int, int>> mySet(positions.begin(), positions.end());
  BOOST_REQUIRE(mySet.size() == positions.size());
}

BOOST_AUTO_TEST_SUITE_END()