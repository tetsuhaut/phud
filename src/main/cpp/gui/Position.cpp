#include "constants/TableConstants.hpp"
#include "entities/Seat.hpp"
#include "gui/Position.hpp"
#include "gui/Rectangle.hpp"
#include "language/limits.hpp" // toSizeT
#include <frozen/unordered_map.h>

[[nodiscard]] static constexpr std::pair<double, double> mkPair(double x, double y) noexcept {
  return std::make_pair(x, y);
}
static constexpr auto ZERO { mkPair(0, 0) };

// TODO: mauvaises positions pour 3
/* The position of seats, if the window size is 1 x 1 */
static constexpr auto NB_SEATS_TO_COEFF {
  frozen::make_unordered_map<Seat, std::array<std::pair<double, double>, TableConstants::MAX_SEATS>>({
    { Seat::seatTwo,  { mkPair(0.5, 0.1),        mkPair(0.5, 0.9),      ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatThree,  { mkPair(0.3333, 0.3333), mkPair(0.6666, 0.3333), mkPair(0.5, 0.9),      ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatFour,  { mkPair(0.25, 0.25),    mkPair(0.75, 0.25),    mkPair(0.75, 0.75),  mkPair(0.25, 0.75),  ZERO, ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatFive,  { mkPair(0.25, 0.25),    mkPair(0.75, 0.25),    mkPair(0.75, 0.75),  mkPair(0.5, 1),      mkPair(0.25, 0.75), ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatSix,  { mkPair(0.2, 0.25),    mkPair(0.5, 0.1),        mkPair(0.8, 0.25),  mkPair(0.8, 0.75),  mkPair(0.5, 0.9),     mkPair(0.2, 0.75), ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatSeven,  { mkPair(0, 0.5),        mkPair(0.25, 0.25),    mkPair(0.5, 0),      mkPair(0.75, 0.25),  mkPair(1, 0.5),     mkPair(0.75, 0.75), mkPair(0.25, 0.75), ZERO, ZERO, ZERO } },
    { Seat::seatEight,  { mkPair(0, 0.5),        mkPair(0.25, 0.25),    mkPair(0.5, 0),      mkPair(0.75, 0.25),  mkPair(1, 0.5),     mkPair(0.75, 0.75), mkPair(0.5, 1), mkPair(0.25, 0.75), ZERO, ZERO } },
    { Seat::seatNine,  { mkPair(0.125, 0.35),   mkPair(0.35, 0.125),   mkPair(0.625, 0.125), mkPair(0.85, 0.35),  mkPair(0.85, 0.635), mkPair(0.625, 0.85), mkPair(0.5, 1), mkPair(0.35, 0.85), mkPair(0.125, 0.635), ZERO } },
    { Seat::seatTen, { mkPair(0.125, 0.35),   mkPair(0.35, 0.125),   mkPair(0.5, 0),      mkPair(0.625, 0.125), mkPair(0.85, 0.35), mkPair(0.85, 0.635), mkPair(0.625, 0.85), mkPair(0.5, 1), mkPair(0.35, 0.85), mkPair(0.125, 0.635) } }
  })
};

namespace {
  [[nodiscard]] std::pair<int, int> calculatePosition(Seat seat, Seat maxSeats, const phud::Rectangle& tablePos) {
    const auto& [coefX, coefY] { NB_SEATS_TO_COEFF.find(maxSeats)->second.at(tableSeat::toArrayIndex(seat)) };
    return { limits::toInt(tablePos.x + coefX * tablePos.w),
             limits::toInt(tablePos.y + coefY * tablePos.h) };
  }

  [[nodiscard]] Seat rotateRelativeToHero(Seat seat, Seat heroSeat, Seat maxSeats) {
    if (heroSeat == Seat::seatUnknown) { return seat; }

    const auto max { tableSeat::toInt(maxSeats) };
    const auto seatTmp { tableSeat::toInt(seat) + max - tableSeat::toInt(heroSeat) };
    return tableSeat::fromInt((seatTmp > max) ? (seatTmp - max) : seatTmp);
  }
} // anonymous namespace

/**
  * Returns the PlayerIndicator position.
  * -1 < seat < tableMaxSeats
  * 1 < tableMaxSeats < 11
  * Builds absolute seat position.
  */
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos) {
  assert(seat <= tableMaxSeats);
  return calculatePosition(seat, tableMaxSeats, tablePos);
}

/**
* Returns the PlayerIndicator position, rotated so that heroSeat is at the bottom
* -1 < seat < tableMaxSeats
* 1 < tableMaxSeats < 11
*/
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat_a,
    Seat heroSeat, Seat tableMaxSeats, const phud::Rectangle& tablePos) {
  assert(seat_a <= tableMaxSeats);
  assert(heroSeat <= tableMaxSeats or heroSeat == Seat::seatUnknown);
  const auto rotatedSeat { rotateRelativeToHero(seat_a, heroSeat, tableMaxSeats) };
  return calculatePosition(rotatedSeat, tableMaxSeats, tablePos);
}
