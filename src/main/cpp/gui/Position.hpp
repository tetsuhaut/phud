#pragma once

#include <utility> // std::pair

// foward declarations
enum class Seat : short;
namespace phud { struct Rectangle; }

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
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat heroSeat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos);
