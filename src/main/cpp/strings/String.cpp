#include "containers/algorithms.hpp" // std::find_if_not, phud::algorithms::*
#include "language/assert.hpp"
#include "strings/String.hpp" // StringView
#include <cctype> // std::isspace
#include <charconv> // std::from_chars
#include <gsl/gsl>

namespace pa = phud::algorithms;

namespace phud::algorithms {
template<typename CONTAINER, typename T>
constexpr void replace(CONTAINER& c, const T& oldValue, const T& newValue) noexcept {
  std::replace(std::begin(c), std::end(c), oldValue, newValue);
}
};

template<typename T>
static inline T toT(StringView s) {
  const auto str { phud::strings::trim(s) };
  T ret { 0 };
  std::from_chars(str.data(), str.data() + s.size(), ret);
  return ret;
}

int phud::strings::toInt(StringView s) { return toT<int>(s); }

std::size_t phud::strings::toSizeT(StringView s) { return toT<std::size_t>(s); }

[[nodiscard]] constexpr static bool isSpace(char c) noexcept {
  return c == ' ' or c == '\f' or c == '\n' or c == '\r' or c == '\t' or c == '\v';
}

StringView phud::strings::trim(StringView s) {
  s.remove_prefix(gsl::narrow_cast<StringView::size_type>(std::distance(s.cbegin(),
                  std::find_if_not(s.cbegin(), s.cend(), isSpace))));
  s.remove_suffix(gsl::narrow_cast<StringView::size_type>(std::distance(s.crbegin(),
                  std::find_if_not(s.crbegin(), s.crend(), isSpace))));
  return s;
}

double phud::strings::toDouble(StringView amount) {
  const auto str { phud::strings::trim(amount) };
  double ret { 0 };
#if defined(_MSC_VER) // std::from_chars is not available for double on gcc 11.2.0 TODO est-ce tjrs d'actualite
  std::from_chars(str.data(), str.data() + amount.size(), ret);
#else
#include <stdio.h>

  try { ret = std::stod(String(str)); }
  catch (const std::invalid_argument&) { return 0.0; } // silent error

#endif
  return ret;
}

String phud::strings::replaceAll(StringView s, char oldC, char newC) {
  String ret { s };
  pa::replace(ret, oldC, newC);
  return ret;
}

/*[[nodiscard]]*/ String phud::strings::replaceAll(StringView s, StringView oldStr,
    StringView newStr) {
  String ret { s };
  auto pos { ret.find(oldStr) };

  while (StringView::npos != pos) { pos = ret.replace(pos, oldStr.size(), newStr).find(oldStr); }

  return ret;
}

// as SQL doesn't like simple quotes in strings
String phud::strings::sanitize(StringView s) {
  return phud::strings::replaceAll(phud::strings::trim(s), '\'', '-');
}

double phud::strings::toAmount(StringView amount) {
  // because of default US locale, the decimal separator must be '.'
  return phud::strings::toDouble(phud::strings::replaceAll(amount, ',', '.'));
}

double phud::strings::toBuyIn(StringView buyIn) {
  String token;
  token.reserve(buyIn.size());
  double ret{ 0.0 };
  pa::forEach(buyIn, [&token, &ret](auto c) {
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