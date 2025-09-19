#include "log/LoggingLevel.hpp"
#include "language/EnumMapper.hpp"

static constexpr auto LOGGING_MAPPER = makeEnumMapper<LoggingLevel, 7>({{
  {LoggingLevel::trace, "trace"}, {LoggingLevel::debug, "debug"},
  {LoggingLevel::info, "info"}, {LoggingLevel::warn, "warning"},
  {LoggingLevel::error, "error"}, {LoggingLevel::critical, "critical"},
  {LoggingLevel::none, "none"}
}});

LoggingLevel toLoggingLevel(std::string_view sv) {
  return LOGGING_MAPPER.fromString(sv);
}

std::string_view toString(LoggingLevel l) {
  return LOGGING_MAPPER.toString(l);
}