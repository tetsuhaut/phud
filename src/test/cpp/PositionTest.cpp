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

BOOST_AUTO_TEST_SUITE(PositionTest)

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

BOOST_AUTO_TEST_CASE(PositionTest_Rotation) {
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatOne, Seat::seatTwo, Seat::seatTwo), Seat::seatTwo);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatTwo, Seat::seatTwo, Seat::seatTwo), Seat::seatOne);

  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatOne, Seat::seatTwo, Seat::seatThree), Seat::seatThree);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatTwo, Seat::seatTwo, Seat::seatThree), Seat::seatOne);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatThree, Seat::seatTwo, Seat::seatThree), Seat::seatTwo);

  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatOne, Seat::seatThree, Seat::seatFive), Seat::seatFour);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatTwo, Seat::seatThree, Seat::seatFive), Seat::seatFive);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatThree, Seat::seatThree, Seat::seatFive), Seat::seatOne);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatFour, Seat::seatThree, Seat::seatFive), Seat::seatTwo);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatFive, Seat::seatThree, Seat::seatFive), Seat::seatThree);

  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatOne, Seat::seatTwo, Seat::seatSix), Seat::seatSix);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatTwo, Seat::seatTwo, Seat::seatSix), Seat::seatOne);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatThree, Seat::seatTwo, Seat::seatSix), Seat::seatTwo);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatFour, Seat::seatTwo, Seat::seatSix), Seat::seatThree);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatFive, Seat::seatTwo, Seat::seatSix), Seat::seatFour);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatSix, Seat::seatTwo, Seat::seatSix), Seat::seatFive);

  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatOne, Seat::seatSeven, Seat::seatNine), Seat::seatFour);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatTwo, Seat::seatSeven, Seat::seatNine), Seat::seatFive);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatThree, Seat::seatSeven, Seat::seatNine), Seat::seatSix);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatFour, Seat::seatSeven, Seat::seatNine), Seat::seatSeven);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatFive, Seat::seatSeven, Seat::seatNine), Seat::seatEight);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatSix, Seat::seatSeven, Seat::seatNine), Seat::seatNine);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatSeven, Seat::seatSeven, Seat::seatNine), Seat::seatOne);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatEight, Seat::seatSeven, Seat::seatNine), Seat::seatTwo);
  BOOST_CHECK_EQUAL(gui::rotateRelativeToHero(Seat::seatNine, Seat::seatSeven, Seat::seatNine), Seat::seatThree);
}

BOOST_AUTO_TEST_SUITE_END()
