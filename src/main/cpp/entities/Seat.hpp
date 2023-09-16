#pragma once
#include "language/assert.hpp"
#include "strings/StringView.hpp"

enum class /*[[nodiscard]]*/ Seat : short {
  seatOne, seatTwo, seatThree, seatFour, seatFive, seatSix, seatSeven,
  seatEight, seatNine, seatTen, seatUnknown
};

namespace tableSeat {
/*
 * Transforms "1" into Seat::SeatOne and so on.
 */
[[nodiscard]] Seat fromString(StringView seatStr);

/*
 * Transforms 0 into Seat::SeatOne and so on.
 */
[[nodiscard]] Seat fromArrayIndex(std::size_t i);

/*
 * Transforms 1 into Seat::SeatOne and so on.
 */
[[nodiscard]] Seat fromInt(int i);

/*
 * Transforms Seat::SeatOne into 1 and so on.
 */
[[nodiscard]] int toInt(Seat seat);

/*
 * Transforms Seat::SeatOne into 1 and so on.
 */
[[nodiscard]] StringView toString(Seat s);

/*
 * Transforms Seat::SeatOne into 0 and so on.
 */
[[nodiscard]] std::size_t toArrayIndex(Seat seat);
}
