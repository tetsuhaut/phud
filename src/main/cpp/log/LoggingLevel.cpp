#include "log/LoggingLevel.hpp"
#include <frozen/string.h>
#include <frozen/unordered_map.h>

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
  frozen::make_unordered_map<LoggingLevel, std::string_view>({
    { LoggingLevel::trace, "trace" },
    { LoggingLevel::debug, "debug" },
    { LoggingLevel::info, "info" },
    { LoggingLevel::warn, "warning" },
    { LoggingLevel::error, "error" },
    { LoggingLevel::critical, "critical" },
    { LoggingLevel::none, "none" }
  })
};

LoggingLevel toLoggingLevel(std::string_view sv) {
  return STRING_TO_ENUM.find(sv)->second;
}

std::string_view toString(LoggingLevel l) {
  return ENUM_TO_STRING.find(l)->second;
}