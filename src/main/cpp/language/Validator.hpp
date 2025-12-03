#pragma once

#include <string_view>
#include <spdlog/fmt/bundled/format.h> // fmt::format

#ifndef NDEBUG
void phudMacroAssert(const char* expression, bool expressionReturnValue, const char* fileName,
                     const char* functionName, int line, std::string_view errorMessage);
#  define phudAssert(Expr, Msg) phudMacroAssert(#Expr, Expr, __FILE__, __func__, __LINE__, Msg)
#else
#  define phudAssert(Expr, Msg)
#endif // NDEBUG

namespace validation {
template <typename T>
void requireNonEmpty(const T& value, std::string_view fieldName) {
  phudAssert(!value.empty(), fmt::format("{} cannot be empty", fieldName));
}

template <typename T>
void requirePositive(T value, std::string_view fieldName) {
  phudAssert(value >= 0, fmt::format("{} must be non-negative", fieldName));
}

template <typename T>
void requireNotNull(T value, std::string_view errorMessage) {
  phudAssert(nullptr != value, errorMessage);
}

template <typename T>
void requireInRange(T value, T min, T max, std::string_view fieldName) {
  phudAssert(value >= min and value <= max,
             fmt::format("{} must be between {} and {}", fieldName, min, max));
}

void require(bool mustBeTrue, std::string_view errorMessage);
} // namespace validation
