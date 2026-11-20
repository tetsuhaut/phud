#pragma once

#include "log/LoggingLevel.hpp"      // std::string_view
#include "strings/StringLiteral.hpp" // concatLiteral
#include <span>                      // std::span

#if defined(_MSC_VER) // removal of specific msvc warnings due to fmt
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027)
#endif // _MSC_VER

#include <spdlog/fmt/bundled/format.h> // fmt::format()

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#endif

/**
 * Logging never throws: it is used in destructors and in other noexcept functions. If something
 * goes wrong while logging (e.g. a memory allocation failure), the message is lost.
 */
class [[nodiscard]] Logger final {
private:
  std::string_view m_name;
  void logMessage(LoggingLevel level, std::string_view msg) const noexcept;

  template <StringLiteral FMT, typename... Args>
  void logFormatted(LoggingLevel level, Args&&... args) const noexcept {
    try {
      // static_assert(allTypesAreFormattable<FMT, Args...>()); // TODO
      // to be constexpr, fmt::format requires a constexpr string as first parameter
#if defined(_MSC_VER)
#  pragma warning(suppress : 26485) // a StringLiteral is a C array of characters
#endif // _MSC_VER
      logMessage(level, fmt::format(FMT.value, std::forward<Args>(args)...));
    } catch (...) {
      // logging must never throw: the message is lost
    }
  }

public:
  constexpr explicit Logger(std::string_view name) noexcept
    : m_name {name} {}

  void trace(std::string_view msg) const noexcept { logMessage(LoggingLevel::trace, msg); }
  void debug(std::string_view msg) const noexcept { logMessage(LoggingLevel::debug, msg); }
  void info(std::string_view msg) const noexcept { logMessage(LoggingLevel::info, msg); }
  void warn(std::string_view msg) const noexcept { logMessage(LoggingLevel::warn, msg); }
  void error(std::string_view msg) const noexcept { logMessage(LoggingLevel::error, msg); }
  void critical(std::string_view msg) const noexcept { logMessage(LoggingLevel::critical, msg); }

  template <StringLiteral FMT, typename... Args>
  void trace(Args&&... args) const noexcept {
    logFormatted<FMT>(LoggingLevel::trace, std::forward<Args>(args)...);
  }

  template <StringLiteral FMT, typename... Args>
  void debug(Args&&... args) const noexcept {
    logFormatted<FMT>(LoggingLevel::debug, std::forward<Args>(args)...);
  }

  template <StringLiteral FMT, typename... Args>
  void info(Args&&... args) const noexcept {
    logFormatted<FMT>(LoggingLevel::info, std::forward<Args>(args)...);
  }

  template <StringLiteral FMT, typename... Args>
  void warn(Args&&... args) const noexcept {
    logFormatted<FMT>(LoggingLevel::warn, std::forward<Args>(args)...);
  }

  template <StringLiteral FMT, typename... Args>
  void error(Args&&... args) const noexcept {
    logFormatted<FMT>(LoggingLevel::error, std::forward<Args>(args)...);
  }

  template <StringLiteral FMT, typename... Args>
  void critical(Args&&... args) const noexcept {
    logFormatted<FMT>(LoggingLevel::critical, std::forward<Args>(args)...);
  }

  /** Call one of those methods before any logging. */
  /**
   * Place holders to use to format messages:
   * %D: date
   * %H: time hour
   * %L: Short log level of the message
   * %M: time minutes
   * %S: time seconds
   * %e: time milliseconds
   * %l: log level
   * %n: name of the logger, as provided in the constructor
   * %t: thread identifier
   * %v: message
   * %z: time zone
   * etc.
   * see https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
   */
  static void setupFileInfoLogging(std::string_view pattern);
  static void setupConsoleWarnLogging(std::string_view pattern);
  static void setupConsoleInfoLogging(std::string_view pattern);
  static void setupConsoleDebugLogging(std::string_view pattern);
  static void shutdownLogging() noexcept;
  static void setLoggingLevel(LoggingLevel l) noexcept;
  [[nodiscard]] static LoggingLevel getCurrentLoggingLevel();
}; // class Logger

// inspired by https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
// see user Andry
// also https://godbolt.org/z/u6s8j3
/**
 * @returns the position of the 1st character of the file name in a path,
 * 0 if the given path ends with a / or \.
 * Ex: C:/a/b/c.txt returns 7, a.txt returns 0
 * @tparam CHAR_TYPE the type of character, e.g. char or wchar or uchar
 * @tparam ARRAY_LENGTH the size of the char array containing the string
 * @param str the char array
 * @param startPos the position in str to start searching for the beginning of the file name
 * @return position of the 1st file name character in str
 */
template <typename CHAR_TYPE, std::size_t ARRAY_LENGTH>
constexpr std::size_t getFileNameOffset(const CHAR_TYPE (&str)[ARRAY_LENGTH]) {
  if constexpr (ARRAY_LENGTH == 1) {
    return 0;
  }
  const std::span<const CHAR_TYPE, ARRAY_LENGTH> buffer {str};
  for (std::size_t i = ARRAY_LENGTH - 1; i > 0; --i) {
    if (('/' == buffer[i]) or ('\\' == buffer[i])) {
      return i + 1;
    }
  }
  return 0;
}

struct [[nodiscard]] LoggingConfig final {
  explicit LoggingConfig(std::string_view pattern) { Logger::setupFileInfoLogging(pattern); }
  LoggingConfig(const LoggingConfig&) = delete;
  LoggingConfig(LoggingConfig&&) = delete;
  LoggingConfig& operator=(const LoggingConfig&) = delete;
  LoggingConfig& operator=(LoggingConfig&&) = delete;
  ~LoggingConfig() { Logger::shutdownLogging(); }
};

// forces the compiler to do a compile time evaluation
namespace utility {
  template <typename T, T v>
  struct [[nodiscard]] ConstexprValue final {
    static constexpr T value = v;
  }; // struct ConstexprValue
} // namespace utility

#define CURRENT_FILE_NAME \
  &__FILE__[::utility::ConstexprValue<std::size_t, getFileNameOffset(__FILE__)>::value]
