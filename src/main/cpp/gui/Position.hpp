#pragma once

#include <utility> // std::pair

// foward declarations
enum class Seat : short;
namespace phud {
struct Rectangle;
}

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
 * @brief Rotates the given seat so that the hero seat always appears at the bottom.<br>
 * At the table, the seat 1 is top left. Since we want the hero to always be at the bottom:
 * - if hero is at seat 1 -> rotate of 6 in 9 max table, 4 in 6 max table, 3 in 5 max table, 2 in 3
 * max table, 0 in 2 max table
 * - if hero is at seat 2 -> rotate of 5 in 9 max table, 3 in 6 max table, 2 in 5 max table, 1 in 3
 * max table, 1 in 2 max table
 * - if hero is at seat 3 -> rotate of 4 in 9 max table, 2 in 6 max table, 1 in 5 max table, 0 in 3
 * max table
 * - if hero is at seat 4 -> rotate of 3 in 9 max table, 1 in 6 max table, 0 in 5 max table,
 * - if hero is at seat 5 -> rotate of 2 in 9 max table, 0 in 6 max table, 4 in 5 max table,
 * - if hero is at seat 6 -> rotate of 1 in 9 max table, 5 in 6 max table
 * - if hero is at seat 7 -> rotate of 0 in 9 max table
 * - if hero is at seat 8 -> rotate of 8 in 9 max table
 * - if hero is at seat 9 -> rotate of 7 in 9 max table
 * @param seat The seat to be rotated must be < maxSeats
 * @param heroSeat The hero seat, must be < maxSeats
 * @param maxSeats The maximum seat number
 * @return The rotated seat
 */
[[nodiscard]] Seat rotateRelativeToHero(Seat seat, Seat heroSeat, Seat maxSeats);
} // namespace gui
