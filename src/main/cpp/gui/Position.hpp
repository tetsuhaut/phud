#pragma once

#include "containers/Pair.hpp"
#include "entities/Seat.hpp"

namespace phud {
struct Rectangle;
}; // namespace phud

[[nodiscard]] Pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat tableMaxSeats,
    const phud::Rectangle& tablePos);

/**
* Builds seat position according to hero's position.
*/
[[nodiscard]] Pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat heroSeat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos);
