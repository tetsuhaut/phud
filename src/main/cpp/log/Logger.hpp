#pragma once

#include "log/LoggingLevel.hpp" // StringView
#include "strings/String.hpp"
#include "strings/StringLiteral.hpp" // concatLiteral

#if defined(_MSC_VER) // removal of specific msvc warnings due to fmt
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027 )
#endif  // _MSC_VER

#include <spdlog/fmt/bundled/format.h> // fmt::format()

#if defined(_MSC_VER)  // end of specific msvc warnings removal
#  pragma warning(pop)
#endif

class [[nodiscard]] Logger final {
private:
  StringView m_name;
  static void traceStr(StringView msg);
  static void debugStr(StringView msg);
  static void infoStr(StringView msg);
  static void warnStr(StringView msg);
  static void errorStr(StringView msg);
  static void criticalStr(StringView msg);

  template<StringLiteral FMT>
  void _trace(auto&& ... args) {
    traceStr(fmt::format(FMT.value, std::forward<decltype(args)>(args)...));
  }

  template<StringLiteral FMT>
  void _debug(auto&& ... args) {
    debugStr(fmt::format(FMT.value, std::forward<decltype(args)>(args)...));
  }

  template<StringLiteral FMT>
  void _info(auto&& ... args) {
    infoStr(fmt::format(FMT.value, std::forward<decltype(args)>(args)...));
  }

  template<StringLiteral FMT>
  void _warn(auto&& ... args) {
    warnStr(fmt::format(FMT.value, std::forward<decltype(args)>(args)...));
  }

  template<StringLiteral FMT>
  void _error(auto&& ... args) {
    errorStr(fmt::format(FMT.value, std::forward<decltype(args)>(args)...));
  }

  template<StringLiteral FMT>
  void _critical(auto&& ... args) {
    criticalStr(fmt::format(FMT.value, std::forward<decltype(args)>(args)...));
  }

public:
  Logger(StringView name) : m_name { name } {}

  // to be constexpr, fmt::format requires a constexpr string as first parameter
  void trace(StringView msg) { traceStr(fmt::format("[{}]: {}", m_name, msg)); }
  void debug(StringView msg) { debugStr(fmt::format("[{}]: {}", m_name, msg)); }
  void info(StringView msg) { infoStr(fmt::format("[{}]: {}", m_name, msg)); }
  void warn(StringView msg) { warnStr(fmt::format("[{}]: {}", m_name, msg)); }
  void error(StringView msg) { errorStr(fmt::format("[{}]: {}", m_name, msg)); }
  void critical(StringView msg) { criticalStr(fmt::format("[{}]: {}", m_name, msg)); }

  template<StringLiteral FMT>
  void trace(auto&& ... args) {
    _trace<concatLiteral("[{}]: ", FMT.value)>(m_name, std::forward<decltype(args)>(args)...);
  }

  template<StringLiteral FMT>
  void debug(auto&& ... args) {
    _debug<concatLiteral("[{}]: ", FMT.value)>(m_name, std::forward<decltype(args)>(args)...);
  }
  template<StringLiteral FMT>
  void info(auto&& ... args) {
    _info<concatLiteral("[{}]: ", FMT.value)>(m_name, std::forward<decltype(args)>(args)...);
  }
  template<StringLiteral FMT>
  void warn(auto&& ... args) {
    _warn<concatLiteral("[{}]: ", FMT.value)>(m_name, std::forward<decltype(args)>(args)...);
  }
  template<StringLiteral FMT>
  void error(auto&& ... args) {
    _error<concatLiteral("[{}]: ", FMT.value)>(m_name, std::forward<decltype(args)>(args)...);
  }
  template<StringLiteral FMT>
  void critical(auto&& ... args) {
    _critical<concatLiteral("[{}]: ", FMT.value)>(m_name, std::forward<decltype(args)>(args)...);
  }
  /** Call one of those methods before any logging. */
  /**
  * Place holders to use to format messages:
  * %D: date
  * %H: time hour
  * %L: dunno lol
  * %M: time minutes
  * %S: time seconds
  * %e: time milliseconds
  * %l: log level
  * %n: name of the logger, as provided in the constructor
  * %t: thread identifier
  * %v: message
  * %z: time zone
  * see https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  */
  static void setupFileInfoLogging(StringView pattern);
  static void setupConsoleWarnLogging(StringView pattern);
  static void setupConsoleDebugLogging(StringView pattern);
  static void shutdownLogging();
  static void setLoggingLevel(LoggingLevel l);
  [[nodiscard]] static LoggingLevel getCurrentLoggingLevel();
}; // class Logger

// inspired from https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
// see user Andry
// also https://godbolt.org/z/u6s8j3
/**
 * @returns the position of the 1st char of the file name in a path,
 * 0 if the given path ends with a / or \.
 * Ex: C:/a/b/c.txt returns 7, a.txt returns 0
 * @param CHAR_TYPE the type of character, e.g. char or wchar or uchar
 * @param ARRAY_LENGTH the size of the char array containing the string
 * @param str the char array
 */
template <typename CHAR_TYPE, std::size_t ARRAY_LENGTH>
constexpr std::size_t getFileNameOffset(const CHAR_TYPE(&str)[ARRAY_LENGTH],
                                        const std::size_t pos = ARRAY_LENGTH - 1) {
  // pos cannot be out of scope
  if ((str[pos] == '/') or (str[pos] == '\\')) { return pos + 1; }

  return (pos > 0) ? getFileNameOffset(str, pos - 1) : 0;
}

constexpr std::size_t getFileNameOffset(auto(& str)[1]) { return 0; }

// forces the compiler to do a compile time evaluation
namespace utility {
template <typename T, T v>
struct [[nodiscard]] ConstexprValue final {
  static constexpr T value = v;
}; // struct ConstexprValue
}; // namespace utility

#define CURRENT_FILE_NAME &__FILE__[::utility::ConstexprValue<std::size_t, getFileNameOffset(__FILE__)>::value]