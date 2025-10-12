#include "TestInfrastructure.hpp"
#include "entities/Seat.hpp"
#include "gui/Rectangle.hpp"
#include "gui/Position.hpp"
#include <array>
#include <print>
#include <ranges> // std::views
#include <set>

namespace {
  void buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat max) {
    const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
    for (int i = 0; i < tableSeat::toArrayIndex(max); ++i) {
      const auto& [x, y] { buildPlayerIndicatorPosition(tableSeat::fromArrayIndex(i), max, tablePosition) };
    }
  }

  void buildRotatedPlayerIndicatorPositionShouldSucceed(Seat max, Seat hero) {
    const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
    for (int i = 0; i < tableSeat::toArrayIndex(max); ++i) {
      const auto& [x, y] { buildPlayerIndicatorPosition(tableSeat::fromArrayIndex(i), hero, max, tablePosition) };
    }
  }
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(PositionTest)

// 0---+ X
// |
// |
// +
// Y

BOOST_AUTO_TEST_CASE(PositionTest_buildAbsolutePlayerIndicatorPositionShouldSucceed) {
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatOne);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatTwo);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatThree);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatFour);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatFive);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatSix);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatSeven);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatEight);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatNine);
  buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat::seatTen);
}

// 0---+ X
// |
// |
// +
// Y

BOOST_AUTO_TEST_CASE(PositionTest_buildPlayerIndicatorPositionShouldSucceed) {
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatOne, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatTwo, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatThree, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatFour, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatFive, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatSix, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatSeven, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatEight, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatNine, Seat::seatOne);
  buildRotatedPlayerIndicatorPositionShouldSucceed(Seat::seatTen, Seat::seatOne);
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

// Tests pour table à 5 joueurs
BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat1_HeroAtBottom) {
  // Table à 5 joueurs, hero au siège 1
  // Le hero doit être en bas (position 4 pour une table à 5)
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatOne };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [heroX, heroY] { buildPlayerIndicatorPosition(heroSeat, heroSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };

  BOOST_CHECK_EQUAL(heroX, expectedX);
  BOOST_CHECK_EQUAL(heroY, expectedY);
}

BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat1_AllSeatsDistinct) {
  // Table à 5 joueurs, hero au siège 1
  // Tous les sièges doivent avoir des positions distinctes
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatOne };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const std::array seats { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive };
  const auto positions { seats
    | std::views::transform([heroSeat, maxSeats, &tablePos](auto seat) {
        return buildPlayerIndicatorPosition(seat, heroSeat, maxSeats, tablePos); })
    | std::ranges::to<std::vector>() };

  std::set<std::pair<int, int>> uniquePositions(positions.begin(), positions.end());
  BOOST_CHECK_EQUAL(uniquePositions.size(), positions.size());
}

BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat3_HeroAtBottom) {
  // Table à 5 joueurs, hero au siège 3
  // Le hero doit être en bas (position 4)
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [heroX, heroY] { buildPlayerIndicatorPosition(heroSeat, heroSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };

  BOOST_CHECK_EQUAL(heroX, expectedX);
  BOOST_CHECK_EQUAL(heroY, expectedY);
}

BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat3_AllSeatsDistinct) {
  // Table à 5 joueurs, hero au siège 3
  // Tous les sièges doivent avoir des positions distinctes
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const std::array seats { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive };
  const auto positions { seats
    | std::views::transform([heroSeat, maxSeats, &tablePos](auto seat) {
        return buildPlayerIndicatorPosition(seat, heroSeat, maxSeats, tablePos); })
    | std::ranges::to<std::vector>() };

  std::set<std::pair<int, int>> uniquePositions(positions.begin(), positions.end());
  BOOST_CHECK_EQUAL(uniquePositions.size(), positions.size());
}

BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat5_HeroAtBottom) {
  // Table à 5 joueurs, hero au siège 5
  // Le hero doit être en bas (position 4)
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatFive };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const auto [heroX, heroY] { buildPlayerIndicatorPosition(heroSeat, heroSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };

  BOOST_CHECK_EQUAL(heroX, expectedX);
  BOOST_CHECK_EQUAL(heroY, expectedY);
}

BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat5_AllSeatsDistinct) {
  // Table à 5 joueurs, hero au siège 5
  // Tous les sièges doivent avoir des positions distinctes
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatFive };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };

  const std::array seats { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive };
  const auto positions { seats
    | std::views::transform([heroSeat, maxSeats, &tablePos](auto seat) {
        return buildPlayerIndicatorPosition(seat, heroSeat, maxSeats, tablePos); })
    | std::ranges::to<std::vector>() };

  std::set<std::pair<int, int>> uniquePositions(positions.begin(), positions.end());
  BOOST_CHECK_EQUAL(uniquePositions.size(), positions.size());
}

BOOST_AUTO_TEST_SUITE_END()
