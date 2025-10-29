#pragma once

#include <string_view>

enum class [[nodiscard]] Seat  : short {
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
  [[nodiscard]] int toInt(Seat seat);

  /*
   * Transforms Seat::SeatOne into 1 and so on.
   */
  [[nodiscard]] std::string_view toString(Seat seat);

  /*
   * Transforms Seat::SeatOne into 0 and so on.
   */
  [[nodiscard]] std::size_t toArrayIndex(Seat seat);

  /*
   * Transforms one seat to the next one.
   */
  [[nodiscard]] Seat next(Seat current, Seat max);
} // namespace tableSeat
