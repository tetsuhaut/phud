#include "containers/algorithms.hpp"
#include "entities/Seat.hpp"
#include "gui/Position.hpp"
#include "gui/Rectangle.hpp"
#include "language/limits.hpp" // toSizeT
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;

[[nodiscard]] static constexpr std::pair<double, double> mkPair(double x, double y) noexcept {
  return std::make_pair(x, y);
}
static constexpr auto ZERO { mkPair(0, 0) };

// TODO: mauvaises positions pour 3
/* The position of seats, if the window size is 1 x 1 */
static constexpr auto NB_SEATS_TO_COEFF {
  frozen::make_unordered_map<Seat, std::array<std::pair<double, double>, 10>>({
    { Seat::seatTwo,  { mkPair(0.5, 0),        mkPair(0.5, 1.0),      ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatThree,  { mkPair(0.3333, 0.3333), mkPair(0.6666, 0.3333), mkPair(0.5, 1),      ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatFour,  { mkPair(0.25, 0.25),    mkPair(0.75, 0.25),    mkPair(0.75, 0.75),  mkPair(0.25, 0.75),  ZERO, ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatFive,  { mkPair(0.25, 0.25),    mkPair(0.75, 0.25),    mkPair(0.75, 0.75),  mkPair(0.5, 1),      mkPair(0.25, 0.75), ZERO, ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatSix,  { mkPair(0.25, 0.25),    mkPair(0.5, 0),        mkPair(0.75, 0.25),  mkPair(0.75, 0.75),  mkPair(0.5, 1),     mkPair(0.25, 0.75), ZERO, ZERO, ZERO, ZERO } },
    { Seat::seatSeven,  { mkPair(0, 0.5),        mkPair(0.25, 0.25),    mkPair(0.5, 0),      mkPair(0.75, 0.25),  mkPair(1, 0.5),     mkPair(0.75, 0.75), mkPair(0.25, 0.75), ZERO, ZERO, ZERO } },
    { Seat::seatEight,  { mkPair(0, 0.5),        mkPair(0.25, 0.25),    mkPair(0.5, 0),      mkPair(0.75, 0.25),  mkPair(1, 0.5),     mkPair(0.75, 0.75), mkPair(0.5, 1), mkPair(0.25, 0.75), ZERO, ZERO } },
    { Seat::seatNine,  { mkPair(0.125, 0.35),   mkPair(0.35, 0.125),   mkPair(0.625, 0.125), mkPair(0.85, 0.35),  mkPair(0.85, 0.635), mkPair(0.625, 0.85), mkPair(0.5, 1), mkPair(0.35, 0.85), mkPair(0.125, 0.635), ZERO } },
    { Seat::seatTen, { mkPair(0.125, 0.35),   mkPair(0.35, 0.125),   mkPair(0.5, 0),      mkPair(0.625, 0.125), mkPair(0.85, 0.35), mkPair(0.85, 0.635), mkPair(0.625, 0.85), mkPair(0.5, 1), mkPair(0.35, 0.85), mkPair(0.125, 0.635) } }
  })
};

/**
* Returns the PlayerIndicator position.
* -1 < seat < tableMaxSeats
* 1 < tableMaxSeats < 11
*/
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos) {
  assert(seat <= tableMaxSeats);
  const auto [coefX, coefY] { pa::getValueFromKey(NB_SEATS_TO_COEFF, tableMaxSeats).at(tableSeat::toArrayIndex(seat)) };
  return { limits::toInt(tablePos.x + coefX * tablePos.w), limits::toInt(tablePos.y + coefY * tablePos.h) };
}

/**
* Returns the PlayerIndicator position.
* -1 < seat < tableMaxSeats
* 1 < tableMaxSeats < 11
*/
// exported for unit testing
/*[[nodiscard]] static inline*/ std::pair<int, int> buildPlayerIndicatorPosition(Seat seat_a,
    Seat heroSeat, Seat tableMaxSeats, const phud::Rectangle& tablePos) {
  assert(seat_a <= tableMaxSeats);
  assert(heroSeat <= tableMaxSeats);
  // hero is always positionned at the bottom of the table
  const auto max { tableSeat::toInt(tableMaxSeats) };
  const auto seatTmp { tableSeat::toInt(seat_a) + max - tableSeat::toInt(heroSeat) };
  const auto seat { tableSeat::fromInt((seatTmp > max) ? (seatTmp - max) : seatTmp) };
  return buildPlayerIndicatorPosition(seat, tableMaxSeats, tablePos);
}
