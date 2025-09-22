#pragma once

#include <string_view>

enum class /*[[nodiscard]]*/ Seat : short {
  seatOne, seatTwo, seatThree, seatFour, seatFive, seatSix, seatSeven,
  seatEight, seatNine, seatTen, seatUnknown
};

namespace tableSeat {
/*
 * Transforms "1" into Seat::SeatOne and so on.
 */
[[nodiscard]] Seat fromString(std::string_view seatStr);

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
[[nodiscard]] constexpr int toInt(Seat seat) {
  return static_cast<int>(seat) + 1;
}

/*
 * Transforms Seat::SeatOne into 1 and so on.
 */
[[nodiscard]] std::string_view toString(Seat s);

/*
 * Transforms Seat::SeatOne into 0 and so on.
 */
[[nodiscard]] constexpr std::size_t toArrayIndex(Seat seat) {
  return static_cast<std::size_t>(seat);
}
} // namespace tableSeat
