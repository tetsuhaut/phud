#include "log/LoggingLevel.hpp"
#include "language/EnumMapper.hpp"

static constexpr auto LOGGING_MAPPER = makeEnumMapper<LoggingLevel>(
    std::pair {LoggingLevel::trace, "trace"}, std::pair {LoggingLevel::debug, "debug"},
    std::pair {LoggingLevel::info, "info"}, std::pair {LoggingLevel::warn, "warning"},
    std::pair {LoggingLevel::error, "error"}, std::pair {LoggingLevel::critical, "critical"},
    std::pair {LoggingLevel::none, "none"});

LoggingLevel toLoggingLevel(std::string_view sv) {
  return LOGGING_MAPPER.fromString(sv);
}

std::string_view toString(LoggingLevel l) {
  return LOGGING_MAPPER.toString(l);
}
