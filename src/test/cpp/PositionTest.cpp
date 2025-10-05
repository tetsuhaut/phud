#include "TestInfrastructure.hpp"
#include "entities/Seat.hpp"
#include "gui/Rectangle.hpp"
#include "gui/Position.hpp"
#include <array>
#include <print>
#include <ranges> // std::views
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
  const std::array seats { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix };
  std::ranges::for_each(seats, [heroSeat, tableMaxSeat, &tablePosition](auto seat) {
    auto [x, y] { buildPlayerIndicatorPosition(seat, heroSeat, tableMaxSeat, tablePosition) };
    BOOST_REQUIRE(tablePosition.x <= x and x <= (tablePosition.x + tablePosition.w));
    BOOST_REQUIRE(50 <= tablePosition.y and (tablePosition.y + tablePosition.h) <= 250);
  });
}

BOOST_AUTO_TEST_CASE(PositionTest_playerIndicatorsShouldHaveDistinctPositions) {
  const auto tablePosition { phud::Rectangle {.x = 50, .y = 50, .w = 300, .h = 200 } };
  const auto heroSeat { Seat::seatThree };
  const auto tableMaxSeat { Seat::seatSix };
  const std::array seats { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix };
  const auto positions { seats 
    | std::views::transform([heroSeat, tableMaxSeat, &tablePosition](auto seat) {
        return buildPlayerIndicatorPosition(seat, heroSeat, tableMaxSeat, tablePosition); })
    | std::ranges::to<std::vector>() };
  std::set<std::pair<int, int>> mySet(positions.begin(), positions.end());
  BOOST_REQUIRE(mySet.size() == positions.size());
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_HeroAtBottom) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège du hero doit être positionné en bas (position 5 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };
  const auto [heroX, heroY] { buildPlayerIndicatorPosition(heroSeat, heroSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { buildPlayerIndicatorPosition(Seat::seatFive, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(heroX, expectedX);
  BOOST_CHECK_EQUAL(heroY, expectedY);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_SeatLeftOfHero) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège 4 doit être à gauche du hero (position 0 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [seat4X, seat4Y] { buildPlayerIndicatorPosition(Seat::seatFour, heroSeat, maxSeats, tablePos) };
  const auto [expected4X, expected4Y] { buildPlayerIndicatorPosition(Seat::seatOne, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(seat4X, expected4X);
  BOOST_CHECK_EQUAL(seat4Y, expected4Y);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_SeatRightOfHero) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège 2 doit être à droite du hero (position 4 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [seat2X, seat2Y] { buildPlayerIndicatorPosition(Seat::seatTwo, heroSeat, maxSeats, tablePos) };
  const auto [expected2X, expected2Y] { buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(seat2X, expected2X);
  BOOST_CHECK_EQUAL(seat2Y, expected2Y);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_SeatOppositeOfHero) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège 0 doit être en haut à gauche (position 3 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [seat0X, seat0Y] { buildPlayerIndicatorPosition(Seat::seatOne, heroSeat, maxSeats, tablePos) };
  const auto [expected0X, expected0Y] { buildPlayerIndicatorPosition(Seat::seatThree, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(seat0X, expected0X);
  BOOST_CHECK_EQUAL(seat0Y, expected0Y);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_NoRotationWhenHeroUnknown) {
  // Test avec hero en position inconnue (pas de rotation)
  const auto maxSeats { Seat::seatSix };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [noRotX, noRotY] { buildPlayerIndicatorPosition(Seat::seatTwo, Seat::seatUnknown, maxSeats, tablePos) };
  const auto [origX, origY] { buildPlayerIndicatorPosition(Seat::seatTwo, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(noRotX, origX);
  BOOST_CHECK_EQUAL(noRotY, origY);
}

BOOST_AUTO_TEST_SUITE_END()
