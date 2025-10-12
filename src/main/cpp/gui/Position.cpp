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
  /* The position of seats, if the window size is 1 x 1 */
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

  [[nodiscard]] std::pair<int, int> calculatePosition(Seat seat, Seat maxSeats, const phud::Rectangle& tablePos) {
    const auto& [coefX, coefY] { NB_SEATS_TO_COEFF.find(maxSeats)->second.at(tableSeat::toArrayIndex(seat)) };
    return { limits::toInt(tablePos.x + coefX * tablePos.w),
             limits::toInt(tablePos.y + coefY * tablePos.h) };
  }

  [[nodiscard]] constexpr Seat rotateRelativeToHero(Seat seat, Seat heroSeat, Seat maxSeats) {
    if (heroSeat == Seat::seatUnknown) { return seat; }

    const auto max { tableSeat::toInt(maxSeats) };
    const auto seatInt { tableSeat::toInt(seat) };
    const auto heroInt { tableSeat::toInt(heroSeat) };

    // Cas spécial : le hero va toujours à la position (max - 1)
    if (seat == heroSeat) {
      return tableSeat::fromInt(max - 1);
    }

    // Cas spécial : le dernier siège (seat == max) va toujours à la position max
    if (seat == maxSeats) {
      return tableSeat::fromInt(max);
    }

    // Pour les sièges après le hero (dans l'ordre circulaire), utilise offset = max
    // Pour les sièges avant le hero, utilise offset = max - 1
    const auto offset { seatInt > heroInt ? max : (max - 1) };
    const auto seatTmp { seatInt - heroInt + offset };

    // Gère les valeurs négatives (si seat < hero avec offset = max - 1)
    const auto adjusted { seatTmp <= 0 ? seatTmp + max : seatTmp };

    // Wrappe si > max
    const auto wrapped { adjusted > max ? adjusted - max : adjusted };

    return tableSeat::fromInt(wrapped);
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
 * Gets the PlayerIndicator position, rotated so that heroSeat is at the bottom.
 * -1 < seat < tableMaxSeats
 * 1 < tableMaxSeats < 11
 * @param seat The seat we want the PlayerIndicator position
 * @param heroSeat The seat occupied by the hero
 * @tableMaxSeats The maximum seat number
 * @tablePos The table window coordinates and dimensions
 * @returns the PlayerIndicator position, rotated so that heroSeat is at the bottom
 */
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat,
    Seat heroSeat, Seat tableMaxSeats, const phud::Rectangle& tablePos) {
  assert(seat <= tableMaxSeats);
  assert(heroSeat <= tableMaxSeats or heroSeat == Seat::seatUnknown);
  const auto rotatedSeat { rotateRelativeToHero(seat, heroSeat, tableMaxSeats) };
  return calculatePosition(rotatedSeat, tableMaxSeats, tablePos);
}
