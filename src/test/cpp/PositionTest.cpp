#include "TestInfrastructure.hpp"
#include "gui/Rectangle.hpp"
#include "gui/Position.hpp"

#include <print>
#include <set>

BOOST_AUTO_TEST_SUITE(PositionTest)

// 0---+ X
// |
// |
// +
// Y

BOOST_AUTO_TEST_CASE(PositionTest_buildAbsolutePlayerIndicatorPositionFor6MAxShouldSucceed) {
  const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
  const auto tableMaxSeat { Seat::seatSix };
  const auto& [x1, y1] { buildPlayerIndicatorPosition(Seat::seatOne, tableMaxSeat, tablePosition) };
  std::print("x1={}, y1={}\n", x1, y1);
  const auto& [x2, y2] { buildPlayerIndicatorPosition(Seat::seatTwo, tableMaxSeat, tablePosition) };
  std::print("x2={}, y2={}\n", x2, y2);
  const auto& [x3, y3] { buildPlayerIndicatorPosition(Seat::seatThree, tableMaxSeat, tablePosition) };
  std::print("x3={}, y3={}\n", x3, y3);
  const auto& [x4, y4] { buildPlayerIndicatorPosition(Seat::seatFour, tableMaxSeat, tablePosition) };
  std::print("x4={}, y4={}\n", x4, y4);
  const auto& [x5, y5] { buildPlayerIndicatorPosition(Seat::seatFive, tableMaxSeat, tablePosition) };
  std::print("x5={}, y5={}\n", x5, y5);
  const auto& [x6, y6] { buildPlayerIndicatorPosition(Seat::seatSix, tableMaxSeat, tablePosition) };
  std::print("x6={}, y6={}\n", x6, y6);
}

// 0---+ X
// |
// |
// +
// Y

BOOST_AUTO_TEST_CASE(PositionTest_buildAbsolutePlayerIndicatorPositionFor5MAxShouldSucceed) {
  const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
  const auto tableMaxSeat { Seat::seatFive };
  const auto& [x1, y1] { buildPlayerIndicatorPosition(Seat::seatOne, tableMaxSeat, tablePosition) };
  std::print("x1={}, y1={}\n", x1, y1);
  const auto& [x2, y2] { buildPlayerIndicatorPosition(Seat::seatTwo, tableMaxSeat, tablePosition) };
  std::print("x2={}, y2={}\n", x2, y2);
  const auto& [x3, y3] { buildPlayerIndicatorPosition(Seat::seatThree, tableMaxSeat, tablePosition) };
  std::print("x3={}, y3={}\n", x3, y3);
  const auto& [x4, y4] { buildPlayerIndicatorPosition(Seat::seatFour, tableMaxSeat, tablePosition) };
  std::print("x4={}, y4={}\n", x4, y4);
  const auto& [x5, y5] { buildPlayerIndicatorPosition(Seat::seatFive, tableMaxSeat, tablePosition) };
  std::print("x5={}, y5={}\n", x5, y5);
}

// 0---+ X
// |
// |
// +
// Y

BOOST_AUTO_TEST_CASE(PositionTest_buildPlayerIndicatorPositionShouldSucceed) {
  const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
  const auto heroSeat { Seat::seatOne };
  const auto tableMaxSeat { Seat::seatSix };
  const auto& [x1, y1] { buildPlayerIndicatorPosition(Seat::seatOne, heroSeat, tableMaxSeat, tablePosition) };
  std::print("x1={}, y1={}\n", x1, y1);
  const auto& [x2, y2] { buildPlayerIndicatorPosition(Seat::seatTwo, heroSeat, tableMaxSeat, tablePosition) };
  std::print("x2={}, y2={}\n", x2, y2);
  const auto& [x3, y3] { buildPlayerIndicatorPosition(Seat::seatThree, heroSeat, tableMaxSeat, tablePosition) };
  std::print("x3={}, y3={}\n", x3, y3);
  const auto& [x4, y4] { buildPlayerIndicatorPosition(Seat::seatFour, heroSeat, tableMaxSeat, tablePosition) };
  std::print("x4={}, y4={}\n", x4, y4);
  const auto& [x5, y5] { buildPlayerIndicatorPosition(Seat::seatFive, heroSeat, tableMaxSeat, tablePosition) };
  std::print("x5={}, y5={}\n", x5, y5);
  const auto& [x6, y6] { buildPlayerIndicatorPosition(Seat::seatSix, heroSeat, tableMaxSeat, tablePosition) };
  std::print("x6={}, y6={}\n", x6, y6);
}

BOOST_AUTO_TEST_CASE(PositionTest_playerIndicatorsShouldBeLocatedInsideTheTableWindow) {
  const auto tablePosition { phud::Rectangle {.x = 50, .y = 50, .w = 300, .h = 200 } };
  const auto heroSeat { Seat::seatThree };
  const auto tableMaxSeat { Seat::seatSix };
  const auto seats = { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix };
  std::ranges::for_each(seats, [&](auto seat) {
    auto [x, y] { buildPlayerIndicatorPosition(seat, heroSeat, tableMaxSeat, tablePosition) };
    BOOST_REQUIRE(tablePosition.x <= x and x <= (tablePosition.x + tablePosition.w));
    BOOST_REQUIRE(50 <= tablePosition.y and (tablePosition.y + tablePosition.h) <= 250);
  });
}

BOOST_AUTO_TEST_CASE(PositionTest_playerIndicatorsShouldHaveDistinctPositions) {
  const auto tablePosition { phud::Rectangle {.x = 50, .y = 50, .w = 300, .h = 200 } };
  const auto heroSeat { Seat::seatThree };
  const auto tableMaxSeat { Seat::seatSix };
  const auto seats = { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix };
  std::vector<std::pair<int, int>> positions;
  positions.reserve(seats.size());
  std::transform(seats.begin(), seats.end(), std::back_inserter(positions),
  [&](auto seat) {
    return buildPlayerIndicatorPosition(seat, heroSeat, tableMaxSeat, tablePosition);
  });
  std::set<std::pair<int, int>> mySet(positions.begin(), positions.end());
  BOOST_REQUIRE(mySet.size() == positions.size());
}

BOOST_AUTO_TEST_SUITE_END()