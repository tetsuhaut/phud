#pragma once

#include "strings/StringView.hpp"
#include <string>

using String = std::string;

namespace phud::strings {
[[nodiscard]] constexpr const char* plural(auto s) noexcept { return s > 1 ? "s" : ""; }
[[nodiscard]] String replaceAll(StringView s, char oldC, char newC);
[[nodiscard]] String replaceAll(StringView s, StringView oldStr, StringView newStr);

/**
 * Replaces ' by - as SQL doesn't like simple quotes in strings.
 */
[[nodiscard]] String sanitize(StringView s);

/**
 * returns the number of char in a char*, not counting the last '\0' char.
 */
template <typename CHAR, std::size_t SIZE>
[[nodiscard]] constexpr std::size_t length(const CHAR(&)[SIZE]) noexcept { return SIZE - 1; }
}; // namespace phud::strings