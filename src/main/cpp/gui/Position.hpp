#pragma once

#include <utility> // std::pair

// foward declaration
namespace phud { struct Rectangle; }
enum class Seat : short;

/**
* Builds absolute seat position.
*/
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat tableMaxSeats,
    const phud::Rectangle& tablePos);

/**
* Builds seat position, given that hero's position is always bottom.
*/
[[nodiscard]] std::pair<int, int> buildPlayerIndicatorPosition(Seat seat, Seat heroSeat,
    Seat tableMaxSeats, const phud::Rectangle& tablePos);
