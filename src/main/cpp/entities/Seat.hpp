#pragma once

#include "constants/TableConstants.hpp"
#include "language/Validator.hpp"
#include <concepts>
#include <cstddef>
#include <string_view>
#include <utility> // std::in_range()

enum class Seat : short {
  seatOne,
  seatTwo,
  seatThree,
  seatFour,
  seatFive,
  seatSix,
  seatSeven,
  seatEight,
  seatNine,
  seatTen,
  seatUnknown
};

namespace tableSeat {
  /*
   * Transforms "1" into Seat::SeatOne and so on.
   */
  [[nodiscard]] Seat fromString(std::string_view seatStr);

  /*
   * Transforms 0 into Seat::SeatOne, 1 into Seat::SeatTwo and so on.
   * Accepts any integral type (int, size_t, ptrdiff_t...).
   */
  template <std::integral T>
  [[nodiscard]] Seat fromArrayIndex(T i) {
    // std::in_range() compares signed and unsigned values safely
    validation::require(std::in_range<int>(i) and 0 <= static_cast<int>(i) and
                            TableConstants::MAX_SEATS > static_cast<int>(i),
                        "Can't find a seat for that value");
    return static_cast<Seat>(i);
  }

  /*
   * Transforms 1 into Seat::SeatOne and so on.
   */
  [[nodiscard]] Seat fromInt(int i);

  /*
   * Transforms Seat::SeatOne into 1 and so on.
   */
  [[nodiscard]] constexpr int toInt(Seat seat) noexcept {
    return static_cast<int>(seat) + 1;
  }

  /*
   * Transforms Seat::SeatOne into 1, Seat::SeatTwo into 2 and so on.
   */
  [[nodiscard]] std::string_view toString(Seat seat);

  /*
   * Transforms Seat::SeatOne into 0, Seat::SeatTwo into 1 and so on.
   */
  [[nodiscard]] constexpr std::size_t toArrayIndex(Seat seat) noexcept {
    return static_cast<std::size_t>(seat);
  }
} // namespace tableSeat
