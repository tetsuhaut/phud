#include "TestInfrastructure.hpp"
#include "containers/algorithms.hpp"
#include "gui/Rectangle.hpp"
#include "gui/Position.hpp"
#include <spdlog/fmt/bundled/format.h>

#include <set>

namespace pa = phud::algorithms;

BOOST_AUTO_TEST_SUITE(PositionTest)

BOOST_AUTO_TEST_CASE(PositionTest_buildPlayerIndicatorPositionShouldSucceed) {
  const auto [x, y] { buildPlayerIndicatorPosition(Seat::seatOne, Seat::seatOne, Seat::seatSix, { .x = 0, .y = 0, .w = 600, .h = 400 }) };
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
  Vector<Pair<int, int>> positions;
  positions.reserve(seats.size());
  pa::forEach(seats, [&tablePosition, &positions](auto seat) {
    positions.push_back(buildPlayerIndicatorPosition(seat, Seat::seatThree, Seat::seatSix,
                        tablePosition));
  });
  std::set<Pair<int, int>> mySet(positions.begin(), positions.end());
  BOOST_REQUIRE(mySet.size() == positions.size());
}

BOOST_AUTO_TEST_SUITE_END()