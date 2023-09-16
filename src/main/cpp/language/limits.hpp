#pragma once

#include "language/clamp-cast.hpp"

namespace limits {
//[[nodiscard]] static constexpr int toInt(std::size_t value) {
//  // by definition 0 <= std::size_t
//  // max int is < to max std::size_t
//  return (value > std::numeric_limits<int>::max()) ? std::numeric_limits<int>::max() :
//         static_cast<int>(value);
//}

[[nodiscard]] static constexpr int toInt(double value) {
  return clamp_cast::clamp_cast<int>(value);
}

[[nodiscard]] static constexpr int toInt(float value) {
  return clamp_cast::clamp_cast<int>(value);
}

[[nodiscard]] static constexpr int toInt(std::ptrdiff_t value) {
  if (value > std::numeric_limits<int>::max()) { return std::numeric_limits<int>::max(); }

  if (value < std::numeric_limits<int>::min()) { return std::numeric_limits<int>::min(); }

  return static_cast<int>(value);
}

[[nodiscard]] static constexpr int toInt(auto) = delete; // forbid other types

[[nodiscard]] static constexpr std::size_t toSizeT(int value) {
  // int can be < 0, std::size_t can't
  return (value < 0) ? 0 : static_cast<std::size_t>(value);
}

[[nodiscard]] static constexpr std::size_t toSizeT(std::ptrdiff_t value) {
  return (value < 0) ? 0 : static_cast<std::size_t>(value);
}

[[nodiscard]] static constexpr std::size_t toSizeT(auto) = delete; // forbid other types
} // namespace limits