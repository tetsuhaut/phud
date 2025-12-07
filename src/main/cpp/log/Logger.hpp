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

class [[nodiscard]] Logger final {
private:
  std::string_view m_name;
  static void traceStr(std::string_view msg);
  static void debugStr(std::string_view msg);
  static void infoStr(std::string_view msg);
  static void warnStr(std::string_view msg);
  static void errorStr(std::string_view msg);
  static void criticalStr(std::string_view msg);

public:
  explicit Logger(std::string_view name)
    : m_name {name} {}

  // to be constexpr, fmt::format requires a constexpr string as first parameter
  void trace(std::string_view msg) { traceStr(fmt::format("[{}]: {}", m_name, msg)); }
  void debug(std::string_view msg) { debugStr(fmt::format("[{}]: {}", m_name, msg)); }
  void info(std::string_view msg) { infoStr(fmt::format("[{}]: {}", m_name, msg)); }
  void warn(std::string_view msg) { warnStr(fmt::format("[{}]: {}", m_name, msg)); }
  void error(std::string_view msg) { errorStr(fmt::format("[{}]: {}", m_name, msg)); }
  void critical(std::string_view msg) { criticalStr(fmt::format("[{}]: {}", m_name, msg)); }

  template <StringLiteral FMT, typename... Args>
  void trace(Args&&... args) {
    // static_assert(allTypesAreFormattable<FMT, Args...>()); // TODO
    trace(fmt::format(FMT.value, std::forward<Args>(args)...));
  }

  template <StringLiteral FMT, typename... Args>
  void debug(Args&&... args) {
    debug(fmt::format(FMT.value, std::forward<Args>(args)...));
  }

  template <StringLiteral FMT, typename... Args>
  void info(Args&&... args) {
    info(fmt::format(FMT.value, std::forward<Args>(args)...));
  }

  template <StringLiteral FMT, typename... Args>
  void warn(Args&&... args) {
    warn(fmt::format(FMT.value, std::forward<Args>(args)...));
  }

  template <StringLiteral FMT, typename... Args>
  void error(Args&&... args) {
    error(fmt::format(FMT.value, std::forward<Args>(args)...));
  }

  template <StringLiteral FMT, typename... Args>
  void critical(Args&&... args) {
    critical(fmt::format(FMT.value, std::forward<Args>(args)...));
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
  static void setupConsoleDebugLogging(std::string_view pattern);
  static void shutdownLogging();
  static void setLoggingLevel(LoggingLevel l);
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
constexpr std::size_t getFileNameOffset(const CHAR_TYPE (&str)[ARRAY_LENGTH],
                                        const std::size_t startPos = ARRAY_LENGTH - 1) {
  if constexpr (ARRAY_LENGTH == 1) {
    return 0;
  }
  const std::span<const CHAR_TYPE, ARRAY_LENGTH> buffer {str};
  // by construction, startPos cannot be out of scope
  if (('/' == buffer[startPos]) or ('\\' == buffer[startPos])) {
    return startPos + 1;
  }
  return (startPos > 0) ? getFileNameOffset(str, startPos - 1) : 0;
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
