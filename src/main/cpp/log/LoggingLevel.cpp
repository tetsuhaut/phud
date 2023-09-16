#include "containers/algorithms.hpp"
#include "log/LoggingLevel.hpp"
#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;

constexpr auto STRING_TO_ENUM {
  frozen::make_unordered_map<frozen::string, LoggingLevel>({
    { "trace", LoggingLevel::trace },
    { "debug", LoggingLevel::debug },
    { "info", LoggingLevel::info },
    { "warning", LoggingLevel::warn },
    { "error", LoggingLevel::error },
    { "critical", LoggingLevel::critical },
    { "none", LoggingLevel::none }
  })
};

constexpr auto ENUM_TO_STRING {
  frozen::make_unordered_map<LoggingLevel, StringView>({
    { LoggingLevel::trace, "trace" },
    { LoggingLevel::debug, "debug" },
    { LoggingLevel::info, "info" },
    { LoggingLevel::warn, "warning" },
    { LoggingLevel::error, "error" },
    { LoggingLevel::critical, "critical" },
    { LoggingLevel::none, "none" }
  })
};

LoggingLevel toLoggingLevel(StringView sv) {
  return pa::getValueFromKey(STRING_TO_ENUM, sv);
}

StringView toString(LoggingLevel l) {
  return pa::getValueFromKey(ENUM_TO_STRING, l);
}