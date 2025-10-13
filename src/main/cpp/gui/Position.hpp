#pragma once

#include <utility> // std::pair

// foward declarations
enum class Seat : short;
namespace phud { struct Rectangle; }

namespace gui {
  /**
    * Returns the PlayerIndicator position.
    * -1 < seat < tableMaxSeats
    * 1 < tableMaxSeats < 11
    * Builds absolute seat position.
    */
  [[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat tableMaxSeats,
      const phud::Rectangle& tablePos);

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
  /*[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat heroSeat,
      Seat tableMaxSeats, const phud::Rectangle& tablePos);*/

  /**
   * @brief Rotates the given seat so that the hero seat always appears at the bottom
   * @param seat The seat to be rotated, must be < maxSeats
   * @param heroSeat The hero seat, must be < maxSeats
   * @param maxSeats The maximum seat number
   * @return The rotated seat
   */
  [[nodiscard]] Seat rotateRelativeToHero(Seat seat, Seat heroSeat, Seat maxSeats);
} // namespace gui