#include "language/assert.hpp"
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "strings/StringUtils.hpp" // std::string_view
#include <gsl/gsl>

#include <cctype> // std::isspace
#include <charconv> // std::from_chars

static Logger LOG { CURRENT_FILE_NAME };

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

/**
 * @returns the double corresponding to the given string.
 * if amount is only digits: classic std::from_chars
 * if amount is totally unparsable, return 0.
 * if amount is a value ending with €, returns the value.
 * examples :
 * toDouble("abc") returns 0.0
 * toDouble("42") returns 42.0
 * toDouble("x42") returns 0.0
 * toDouble("42aaa") returns 42.0
 * toDouble("42€") returns 42.0
 */
double phud::strings::toDouble(std::string_view amount) {
  const auto str { phud::strings::trim(amount) };
  double ret { 0 };
  const auto result { std::from_chars(str.data(), str.data() + amount.size(), ret) };

  if (result.ec == std::errc::result_out_of_range) {
    LOG.error<"phud::strings::toDouble({})">(amount);
    LOG.error<"amount in double: {}">(ret);
    LOG.error<"Number of treated characters: {}">(result.ptr - str.data());
    LOG.error<"Out of range value">();
    std::exit(8);
  }
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