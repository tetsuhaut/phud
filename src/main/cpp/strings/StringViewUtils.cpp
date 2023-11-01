#include "language/assert.hpp"
#include "strings/StringUtils.hpp" // std::string_view
#include <gsl/gsl>

#include <cctype> // std::isspace
#include <charconv> // std::from_chars

template<typename T>
static inline T toT(std::string_view s) {
  const auto str { phud::strings::trim(s) };
  T ret { 0 };
  std::from_chars(str.data(), str.data() + s.size(), ret);
  return ret;
}

int phud::strings::toInt(std::string_view s) { return toT<int>(s); }

std::size_t phud::strings::toSizeT(std::string_view s) { return toT<std::size_t>(s); }

[[nodiscard]] constexpr static bool isSpace(char c) noexcept {
  return c == ' ' or c == '\f' or c == '\n' or c == '\r' or c == '\t' or c == '\v';
}

std::string_view phud::strings::trim(std::string_view s) {
  s.remove_prefix(gsl::narrow_cast<std::string_view::size_type>(std::distance(s.cbegin(),
                  std::find_if_not(s.cbegin(), s.cend(), isSpace))));
  s.remove_suffix(gsl::narrow_cast<std::string_view::size_type>(std::distance(s.crbegin(),
                  std::find_if_not(s.crbegin(), s.crend(), isSpace))));
  return s;
}

double phud::strings::toDouble(std::string_view amount) {
  const auto str { phud::strings::trim(amount) };
  double ret { 0 };
#if defined(_MSC_VER) // std::from_chars is not available for double on gcc 11.2.0 TODO est-ce tjrs d'actualite
  std::from_chars(str.data(), str.data() + amount.size(), ret);
#else
#include <stdio.h>

  try { ret = std::stod(std::string(str)); }
  catch (const std::invalid_argument&) { return 0.0; } // silent error

#endif
  return ret;
}

std::string phud::strings::replaceAll(std::string_view s, char oldC, char newC) {
  std::string ret { s };
  std::replace(std::begin(ret), std::end(ret), oldC, newC);
  return ret;
}

/*[[nodiscard]]*/ std::string phud::strings::replaceAll(std::string_view s, std::string_view oldStr,
    std::string_view newStr) {
  std::string ret { s };
  auto pos { ret.find(oldStr) };

  while (std::string_view::npos != pos) { pos = ret.replace(pos, oldStr.size(), newStr).find(oldStr); }

  return ret;
}

// as SQL doesn't like simple quotes in strings
std::string phud::strings::sanitize(std::string_view s) {
  return phud::strings::replaceAll(phud::strings::trim(s), '\'', '-');
}

double phud::strings::toAmount(std::string_view amount) {
  // because of default US locale, the decimal separator must be '.'
  return phud::strings::toDouble(phud::strings::replaceAll(amount, ',', '.'));
}

double phud::strings::toBuyIn(std::string_view buyIn) {
  std::string token;
  token.reserve(buyIn.size());
  double ret{ 0.0 };
  std::ranges::for_each(buyIn, [&token, &ret](auto c) {
    switch (c) {
      case ',': { token += '.'; } break;

      case '.': [[fallthrough]];

      case '0': [[fallthrough]];

      case '1': [[fallthrough]];

      case '2': [[fallthrough]];

      case '3': [[fallthrough]];

      case '4': [[fallthrough]];

      case '5': [[fallthrough]];

      case '6': [[fallthrough]];

      case '7': [[fallthrough]];

      case '8': [[fallthrough]];

      case '9': { token.push_back(c); } break;

      case '+': { ret += phud::strings::toDouble(token); token.clear(); } break;

      default: break;
    }
  });

  if (!token.empty()) { ret += phud::strings::toDouble(token); }

  return ret;
}