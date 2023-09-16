#pragma once

#include <string_view>

/**
 * StringView replaces the const String& parameter transmission.
 * It should be transmitted by copy, being a better const char*.
 */
using StringView = std::string_view;


namespace phud::strings {
[[nodiscard]] double toDouble(StringView s);
[[nodiscard]] int toInt(StringView s);
[[nodiscard]] std::size_t toSizeT(StringView s);
[[nodiscard]] constexpr bool contains(StringView s, char c) noexcept { return StringView::npos != s.find(c); }
[[nodiscard]] constexpr bool contains(StringView contains, StringView searched) noexcept { return StringView::npos != contains.find(searched); }
[[nodiscard]] StringView trim(StringView s);
[[nodiscard]] double toAmount(StringView amount);
[[nodiscard]] double toBuyIn(StringView buyIn);
}

[[nodiscard]] inline bool notFound(StringView::size_type st) { return StringView::npos == st; }