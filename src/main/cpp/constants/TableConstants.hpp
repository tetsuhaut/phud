#pragma once

#include <cstddef> // std::size_t

namespace TableConstants {
  static constexpr std::size_t MAX_SEATS { 10 };
  static constexpr std::size_t MAX_CARDS { 5 };
  static constexpr std::size_t MIN_SEAT_NUMBER { 1 };
  static constexpr auto MAX_SEAT_NUMBER { 10 };
  static constexpr auto DEFAULT_X_COORD { 0.0 };
  static constexpr auto DEFAULT_Y_COORD { 0.0 };
} // namespace TableConstants
