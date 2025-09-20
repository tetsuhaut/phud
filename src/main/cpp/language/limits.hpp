#pragma once

#if defined(__MINGW32__) // removal of specific gcc warnings due to clamp-cast
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif  // __MINGW32__

#include <thirdParties/cpp-clamp-cast/clamp-cast.hpp>

#if defined(__MINGW32__) // end of specific gcc warnings removal
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

#include <cstddef> // std::ptrdiff_t
#include <concepts> // std::same_as

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

template<typename T> requires(std::same_as<T, int> or std::same_as<T, std::ptrdiff_t>)
[[nodiscard]] static constexpr std::size_t toSizeT(T value) {
  // std::size_t can't be < 0
  return (value < 0) ? 0 : static_cast<std::size_t>(value);
}
} // namespace limits