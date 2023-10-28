#pragma once

#include "entities/Seat.hpp"

#include <utility> // std::pair

namespace phud {
struct Rectangle;
}; // namespace phud

[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat tableMaxSeats,
    const phud::Rectangle& tablePos);

/**
* Builds seat position according to hero's position.
*/
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat heroSeat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos);
