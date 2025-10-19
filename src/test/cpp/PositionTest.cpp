#include "TestInfrastructure.hpp"
#include "entities/Seat.hpp"
#include "gui/Rectangle.hpp"
#include "gui/Position.hpp"
#include <array>
#include <ostream>
#include <print>
#include <ranges> // std::views
#include <set>

/**
 * To be able to use the boost unit_test API with the Seat enum, we have to provide this.
 * Note: it has to be in the global scope, putting it in the anonymous namespace won't work
 */
static std::ostream& operator<<(std::ostream& os, Seat s) {
  return os << tableSeat::toString(s);
}

namespace {
  void buildAbsolutePlayerIndicatorPositionShouldSucceed(Seat max) {
    const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
    for (int i = 0; i < tableSeat::toArrayIndex(max); ++i) {
      const auto& [x, y] { gui::buildPlayerIndicatorPosition(tableSeat::fromArrayIndex(i), max, tablePosition) };
    }
  }

  void buildRotatedPlayerIndicatorPositionShouldSucceed(Seat max, Seat hero) {
    const auto tablePosition { phud::Rectangle{.x = 0, .y = 0, .w = 600, .h = 400 } };
    for (int i = 0; i < tableSeat::toArrayIndex(max); ++i) {
      const auto rotatedSeat { gui::rotateRelativeToHero(tableSeat::fromArrayIndex(i), hero, max) };
      const auto& [x, y] { gui::buildPlayerIndicatorPosition(rotatedSeat, max, tablePosition) };
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
    const auto rotatedSeat { gui::rotateRelativeToHero(seat, heroSeat, tableMaxSeat) };
    auto [x, y] { gui::buildPlayerIndicatorPosition(rotatedSeat, tableMaxSeat, tablePosition) };
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
        const auto rotatedSeat { gui::rotateRelativeToHero(seat, heroSeat, tableMaxSeat) };
        return gui::buildPlayerIndicatorPosition(rotatedSeat, tableMaxSeat, tablePosition); })
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
  const auto rotatedSeat { gui::rotateRelativeToHero(heroSeat, heroSeat, maxSeats) };
  const auto [heroX, heroY] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { gui::buildPlayerIndicatorPosition(Seat::seatFive, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(heroX, expectedX);
  BOOST_CHECK_EQUAL(heroY, expectedY);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_SeatLeftOfHero) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège 4 doit être à gauche du hero (position 0 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };
  const auto rotatedSeat { gui::rotateRelativeToHero(Seat::seatFour, heroSeat, maxSeats) };
  const auto [seat4X, seat4Y] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expected4X, expected4Y] { gui::buildPlayerIndicatorPosition(Seat::seatOne, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(seat4X, expected4X);
  BOOST_CHECK_EQUAL(seat4Y, expected4Y);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_SeatRightOfHero) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège 2 doit être à droite du hero (position 4 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };
  const auto rotatedSeat { gui::rotateRelativeToHero(Seat::seatTwo, heroSeat, maxSeats) };
  const auto [seat2X, seat2Y] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expected2X, expected2Y] { gui::buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(seat2X, expected2X);
  BOOST_CHECK_EQUAL(seat2Y, expected2Y);
}

BOOST_AUTO_TEST_CASE(PositionTest_testRotatePlayerIndicatorPosition_SeatOppositeOfHero) {
  // Table à 6 joueurs, hero au siège 3
  // Le siège 0 doit être en haut à gauche (position 3 dans le layout)
  const auto maxSeats { Seat::seatSix };
  const auto heroSeat { Seat::seatThree };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };
  const auto rotatedSeat { gui::rotateRelativeToHero(Seat::seatOne, heroSeat, maxSeats) };
  const auto [seat0X, seat0Y] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expected0X, expected0Y] { gui::buildPlayerIndicatorPosition(Seat::seatThree, maxSeats, tablePos) };
  BOOST_CHECK_EQUAL(seat0X, expected0X);
  BOOST_CHECK_EQUAL(seat0Y, expected0Y);
}

// Tests pour table à 5 joueurs
BOOST_AUTO_TEST_CASE(PositionTest_Table5_HeroAtSeat1_HeroAtBottom) {
  // Table à 5 joueurs, hero au siège 1
  // Le hero doit être en bas (position 4 pour une table à 5)
  const auto maxSeats { Seat::seatFive };
  const auto heroSeat { Seat::seatOne };
  const phud::Rectangle tablePos { 0, 0, 600, 400 };
  const auto rotatedSeat { gui::rotateRelativeToHero(heroSeat, heroSeat, maxSeats) };
  const auto [heroX, heroY] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { gui::buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };

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
        const auto rotatedSeat { gui::rotateRelativeToHero(seat, heroSeat, maxSeats) };
        return gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos); })
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
  const auto rotatedSeat { gui::rotateRelativeToHero(heroSeat, heroSeat, maxSeats) };
  const auto [heroX, heroY] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { gui::buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };

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
        const auto rotatedSeat { gui::rotateRelativeToHero(seat, heroSeat, maxSeats) };
        return gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos); })
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
  const auto rotatedSeat { gui::rotateRelativeToHero(heroSeat, heroSeat, maxSeats) };
  const auto [heroX, heroY] { gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos) };
  const auto [expectedX, expectedY] { gui::buildPlayerIndicatorPosition(Seat::seatFour, maxSeats, tablePos) };

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
        const auto rotatedSeat { gui::rotateRelativeToHero(seat, heroSeat, maxSeats) };
        return gui::buildPlayerIndicatorPosition(rotatedSeat, maxSeats, tablePos); })
    | std::ranges::to<std::vector>() };

  std::set<std::pair<int, int>> uniquePositions(positions.begin(), positions.end());
  BOOST_CHECK_EQUAL(uniquePositions.size(), positions.size());
}

BOOST_AUTO_TEST_CASE(PositionTest_NoRotationNeededIfHeroIsAlreadyAtTheBottom) {
  /**
  * -if hero is at seat 1: rotate of 6 in 9 max table, 4 in 6 max table, 3 in 5 max table, 2 in 3 max table, 0 in 2 max table
  * -if hero is at seat 2: rotate of 5 in 9 max table, 3 in 6 max table, 2 in 5 max table, 1 in 3 max table, 1 in 2 max table
  * -if hero is at seat 3: rotate of 4 in 9 max table, 2 in 6 max table, 1 in 5 max table, 0 in 3 max table
  * -if hero is at seat 4: rotate of 3 in 9 max table, 1 in 6 max table, 0 in 5 max table,
  * -if hero is at seat 5: rotate of 2 in 9 max table, 0 in 6 max table, 4 in 5 max table,
  * -if hero is at seat 6: rotate of 1 in 9 max table, 5 in 6 max table
  * -if hero is at seat 7: rotate of 0 in 9 max table
  * -if hero is at seat 8: rotate of 8 in 9 max table
  * -if hero is at seat 9: rotate of 7 in 9 max table
  */
  for (const auto seat : {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix, Seat::seatSeven, Seat::seatEight, Seat::seatNine }) {
    BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(seat, Seat::seatSeven, Seat::seatNine), seat);
  }
  for (const auto seat : { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix }) {
    BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(seat, Seat::seatFive, Seat::seatSix), seat);
  }
  for (const auto seat : { Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive }) {
    BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(seat, Seat::seatFour, Seat::seatFive), seat);
  }
  for (const auto seat : { Seat::seatOne, Seat::seatTwo, Seat::seatThree }) {
    BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(seat, Seat::seatThree, Seat::seatThree), seat);
  }
  for (const auto seat : { Seat::seatOne, Seat::seatTwo}) {
    BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(seat, Seat::seatTwo, Seat::seatTwo), seat);
  }
}

BOOST_AUTO_TEST_SUITE_END()
