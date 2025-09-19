#pragma once

#include "language/assert.hpp"
#include <string_view>
#include <spdlog/formatter.h> // fmt::format

namespace validation {
  template<typename T>
  void requireNonEmpty(const T& value, std::string_view fieldName) {
    phudAssert(!value.empty(), fmt::format("{} cannot be empty", fieldName));
  }

  template<typename T>
  void requirePositive(T value, std::string_view fieldName) {
    phudAssert(value >= 0, fmt::format("{} must be non-negative", fieldName));
  }

  template<typename T>
  void requireInRange(T value, T min, T max, std::string_view fieldName) {
    phudAssert(value >= min and value <= max,
               fmt::format("{} must be between {} and {}", fieldName, min, max));
  }
}