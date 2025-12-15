#include "log/Logger.hpp"          // CURRENT_FILE_NAME
#include "strings/StringUtils.hpp" // std::string_view
#include <gsl/gsl>
#include <spdlog/fmt/bundled/format.h> // fmt::format
#include <charconv>                    // std::from_chars
#include <span>                        // std::span
#include <type_traits>                 // std::is_arithmetic_v, std::is_same_v
#include <stdexcept>                   // std::invalid_argument

static Logger& LOG() {
  static auto logger = Logger(CURRENT_FILE_NAME);
  return logger;
}

namespace {
template <typename T>
  requires std::is_arithmetic_v<T> and (not std::is_same_v<T, bool>)
[[nodiscard]] T fromStringView(std::string_view s) {
  const auto str = phud::strings::trim(s);
  T result {};
  if (str.empty()) {
    return result;
  }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

  const auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

#ifdef __clang__
#pragma clang diagnostic pop
#endif

  if (ec != std::errc {}) {
    throw std::invalid_argument(fmt::format("Failed to convert '{}' to {}", s, typeid(T).name()));
  }
  return result;
}

[[nodiscard]] constexpr bool isNumericCharOrDot(char c) noexcept {
  return (c >= '0' and c <= '9') or c == '.';
}

[[nodiscard]] constexpr char normalizeDecimalSeparator(char c) noexcept {
  return (c == ',') ? '.' : c;
}

[[nodiscard]] double processToken(std::string_view token) {
  return token.empty() ? 0.0 : phud::strings::toDouble(token);
}
} // anonymous namespace

int phud::strings::toInt(std::string_view s) {
  return fromStringView<int>(s);
}

std::size_t phud::strings::toSizeT(std::string_view s) {
  return fromStringView<std::size_t>(s);
}

[[nodiscard]] constexpr static bool isSpace(char c) noexcept {
  return c == ' ' or c == '\f' or c == '\n' or c == '\r' or c == '\t' or c == '\v';
}

std::string_view phud::strings::trim(std::string_view s) {
  s.remove_prefix(gsl::narrow_cast<std::string_view::size_type>(
      std::distance(s.cbegin(), std::ranges::find_if_not(s, isSpace))));
  s.remove_suffix(gsl::narrow_cast<std::string_view::size_type>(
      std::distance(s.crbegin(), std::ranges::find_if_not(s.crbegin(), s.crend(), isSpace))));
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
  const auto str = phud::strings::trim(amount);
  double ret = 0;
  const std::span<const char> buffer = {str.data(), str.size()};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

  if (const auto [ptr, ec] = std::from_chars(buffer.data(), buffer.data() + buffer.size(), ret);
      ec == std::errc::result_out_of_range) {
    LOG().error<"phud::strings::toDouble({})">(amount);
    LOG().error<"amount in double: {}">(ret);
    LOG().error<"Number of treated characters: {}">(ptr - str.data());
    LOG().error<"Out of range value">();
    return 0;
  }

#ifdef __clang__
#pragma clang diagnostic pop
#endif

  return ret;
}

std::string phud::strings::replaceAll(std::string_view s, char oldC, char newC) {
  std::string ret(s);
  std::ranges::replace(ret, oldC, newC);
  return ret;
}

/*[[nodiscard]]*/ std::string phud::strings::replaceAll(std::string_view s, std::string_view oldStr,
                                                        std::string_view newStr) {
  std::string ret(s);
  auto pos = ret.find(oldStr);

  while (std::string_view::npos != pos) {
    pos = ret.replace(pos, oldStr.size(), newStr).find(oldStr);
  }

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
  auto result = 0.0;

  std::ranges::for_each(buyIn, [&token, &result](char c) {
    if (c == '+') {
      result += processToken(token);
      token.clear();
    } else if (isNumericCharOrDot(c)) {
      token.push_back(normalizeDecimalSeparator(c));
    } else if (',' == c) {
      token += '.';
    }
    // ignore other characters (like '€', spaces, etc.)
  });

  result += processToken(token);
  return result;
}
