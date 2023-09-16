#pragma once
#include "strings/StringView.hpp"

enum class /*[[nodiscard]]*/ LoggingLevel : short {
  none,
  trace,
  debug,
  info,
  warn,
  error,
  critical
};

[[nodiscard]] LoggingLevel toLoggingLevel(StringView sv);

[[nodiscard]] StringView toString(LoggingLevel l);