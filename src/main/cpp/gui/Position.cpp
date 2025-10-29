#include "constants/TableConstants.hpp"
#include "entities/Seat.hpp"
#include "gui/Position.hpp"
#include "gui/Rectangle.hpp"
#include "language/limits.hpp" // toSizeT
#include <frozen/unordered_map.h>


namespace {
  [[nodiscard]] constexpr std::pair<double, double> mkPair(double x, double y) noexcept {
    return std::make_pair(x, y);
  }
  constexpr auto ZERO { mkPair(0, 0) };

  // TODO: mauvaises positions pour 3
  /**
   * The position of seats, if the window size is 1 x 1
   * seat one is considered top left
  */
  constexpr auto NB_SEATS_TO_COEFF {
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

  [[nodiscard]] constexpr std::pair<int, int> calculatePosition(Seat seat, Seat maxSeats, const phud::Rectangle& tablePos) {
    const auto& [coefX, coefY] { NB_SEATS_TO_COEFF.find(maxSeats)->second.at(tableSeat::toArrayIndex(seat)) };
    return { limits::toInt(tablePos.x + coefX * tablePos.w),
             limits::toInt(tablePos.y + coefY * tablePos.h) };
  }

  constexpr Seat rotate(Seat tobeRotated, int nbClicks, Seat maxSeats) {
    for (int i = 0; i < nbClicks; ++i) {
      tobeRotated = tableSeat::next(tobeRotated, maxSeats);
    }
    return tobeRotated;
  }
} // anonymous namespace

/**
* table de 2 : siège 1 en bas au milieu
* table de 3 : siège 1 en bas au milieu
* table de 5 : siège 1 en bas au milieu
* table de 6 : siège 1 en bas au milieu
* -if hero is at seat 1: rotate of 6 in 9 max table, 4 in 6 max table, 3 in 5 max table, 2 in 3 max table, 1 in 2 max table
* -if hero is at seat 2: rotate of 5 in 9 max table, 3 in 6 max table, 2 in 5 max table, 1 in 3 max table, 0 in 2 max table
* -if hero is at seat 3: rotate of 4 in 9 max table, 2 in 6 max table, 1 in 5 max table, 0 in 3 max table
* -if hero is at seat 4: rotate of 3 in 9 max table, 1 in 6 max table, 0 in 5 max table,
* -if hero is at seat 5: rotate of 2 in 9 max table, 0 in 6 max table, 4 in 5 max table,
* -if hero is at seat 6: rotate of 1 in 9 max table, 5 in 6 max table
* -if hero is at seat 7: rotate of 0 in 9 max table
* -if hero is at seat 8: rotate of 8 in 9 max table
* -if hero is at seat 9: rotate of 7 in 9 max table
*/ 
/*[[nodiscard]]*/ Seat gui::rotateRelativeToHero(Seat seat, Seat heroSeat, Seat maxSeats) {
  assert(seat <= maxSeats);
  assert(heroSeat <= maxSeats);
  assert(Seat::seatUnknown != heroSeat);
  assert(Seat::seatUnknown != seat);
  assert(Seat::seatUnknown != maxSeats);

  if (Seat::seatOne == heroSeat) {
    return seat;
  }

  const auto nbClicks { tableSeat::toInt(maxSeats) - tableSeat::toInt(heroSeat) + 1};
  return rotate(seat, nbClicks, maxSeats);
}

/**
  * Returns the PlayerIndicator position.
  * -1 < seat < tableMaxSeats
  * 1 < tableMaxSeats < 11
  * Builds absolute seat position.
  */
[[nodiscard]] std::pair<int, int> gui::buildPlayerIndicatorPosition(Seat seat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos) {
  assert(seat <= tableMaxSeats);
  return calculatePosition(seat, tableMaxSeats, tablePos);
}
