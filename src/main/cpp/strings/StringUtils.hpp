#pragma once

#include <string>
#include <string_view>

namespace phud::strings {
[[nodiscard]] constexpr const char* plural(auto s) noexcept { return s > 1 ? "s" : ""; }
[[nodiscard]] std::string replaceAll(std::string_view s, char oldC, char newC);
[[nodiscard]] std::string replaceAll(std::string_view s, std::string_view oldStr,
                                     std::string_view newStr);

/**
 * Replaces ' by - as SQL doesn't like simple quotes in strings.
 */
[[nodiscard]] std::string sanitize(std::string_view s);

/**
 * returns the number of char in a char*, not counting the last '\0' char.
 */
template <typename CHAR, std::size_t SIZE>
[[nodiscard]] constexpr std::size_t length(const CHAR(&)[SIZE]) noexcept { return SIZE - 1; }

[[nodiscard]] double toDouble(std::string_view s);
[[nodiscard]] int toInt(std::string_view s);
[[nodiscard]] std::size_t toSizeT(std::string_view s);
[[nodiscard]] constexpr bool contains(std::string_view s, char c) noexcept { return std::string_view::npos != s.find(c); }
[[nodiscard]] constexpr bool contains(std::string_view contains,
                                      std::string_view searched) noexcept { return std::string_view::npos != contains.find(searched); }
[[nodiscard]] std::string_view trim(std::string_view s);
[[nodiscard]] double toAmount(std::string_view amount);
[[nodiscard]] double toBuyIn(std::string_view buyIn);
} // namespace phud::strings

[[nodiscard]] inline bool notFound(std::string_view::size_type st) { return std::string_view::npos == st; }