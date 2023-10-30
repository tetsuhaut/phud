#pragma once

#include <string_view>

enum class /*[[nodiscard]]*/ LoggingLevel : short {
  none,
  trace,
  debug,
  info,
  warn,
  error,
  critical
};

[[nodiscard]] LoggingLevel toLoggingLevel(std::string_view sv);

[[nodiscard]] std::string_view toString(LoggingLevel l);