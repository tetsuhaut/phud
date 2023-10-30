#include "containers/algorithms.hpp"
#include "log/Logger.hpp" // std::string_view, String

#include <frozen/unordered_map.h>

#include <spdlog/async.h> // spdlog::async_factory
#include <spdlog/sinks/basic_file_sink.h> // spdlog::basic_logger_mt
#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_mt
#include <spdlog/spdlog.h> // spdlog::level::level_enum, spdlog::logger

#include <memory> // std::shared_ptr

using LegacyLoggingLevel = spdlog::level::level_enum;
namespace pa = phud::algorithms;
namespace {
std::shared_ptr<spdlog::logger> globalLogger;

constexpr auto LEGACCY_LOGGING_LEVEL_TO_LOGGING_LEVEL {
  frozen::make_unordered_map<LegacyLoggingLevel, LoggingLevel>({
    { LegacyLoggingLevel::critical, LoggingLevel::critical },
    { LegacyLoggingLevel::debug, LoggingLevel::debug },
    { LegacyLoggingLevel::err, LoggingLevel::error },
    { LegacyLoggingLevel::info, LoggingLevel::info },
    { LegacyLoggingLevel::trace, LoggingLevel::trace },
    { LegacyLoggingLevel::warn, LoggingLevel::warn },
    { LegacyLoggingLevel::off, LoggingLevel::none }
  })
};

constexpr auto LOGGING_LEVEL_TO_LEGACCY_LOGGING_LEVEL {
  frozen::make_unordered_map<LoggingLevel, LegacyLoggingLevel>({
    { LoggingLevel::critical, LegacyLoggingLevel::critical },
    { LoggingLevel::debug, LegacyLoggingLevel::debug },
    { LoggingLevel::error, LegacyLoggingLevel::err },
    { LoggingLevel::info, LegacyLoggingLevel::info },
    { LoggingLevel::trace, LegacyLoggingLevel::trace },
    { LoggingLevel::warn, LegacyLoggingLevel::warn },
    { LoggingLevel::none, LegacyLoggingLevel::off }
  })
};
} // anonymous namespace

[[nodiscard]] static inline LoggingLevel toLoggingLevel(LegacyLoggingLevel l) {
  return pa::getValueFromKey(LEGACCY_LOGGING_LEVEL_TO_LOGGING_LEVEL, l);
}

[[nodiscard]] static inline LegacyLoggingLevel toLegacyLoggingLevel(LoggingLevel l) {
  return pa::getValueFromKey(LOGGING_LEVEL_TO_LEGACCY_LOGGING_LEVEL, l);
}

/*[[nodiscard]] static*/ LoggingLevel Logger::getCurrentLoggingLevel() {
  return toLoggingLevel(globalLogger->level());
}

/*static*/ void Logger::setLoggingLevel(LoggingLevel l) {
  globalLogger->set_level(toLegacyLoggingLevel(l));
}

/*static*/ void Logger::shutdownLogging() {
  globalLogger->set_level(LegacyLoggingLevel::off);
  spdlog::drop_all();
}

void Logger::traceStr(std::string_view msg) { globalLogger->trace(msg); }
void Logger::debugStr(std::string_view msg) { globalLogger->debug(msg); }
void Logger::infoStr(std::string_view msg) { globalLogger->info(msg); globalLogger->flush(); }
void Logger::warnStr(std::string_view msg) { globalLogger->warn(msg); globalLogger->flush(); }
void Logger::errorStr(std::string_view msg) { globalLogger->error(msg); globalLogger->flush(); }
void Logger::criticalStr(std::string_view msg) { globalLogger->critical(msg); globalLogger->flush(); }

/*static*/ void Logger::setupFileInfoLogging(std::string_view pattern) {
  globalLogger = spdlog::basic_logger_mt<spdlog::async_factory>("fileInfoLogger", "log.txt");
  //globalLogger = spdlog::basic_logger_mt<spdlog::synchronous_factory>("fileInfoLogger", "log.txt");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::info);
}

/*static*/ void Logger::setupConsoleWarnLogging(std::string_view pattern) {
  globalLogger = spdlog::stdout_color_mt<spdlog::async_factory>("consoleWarnLogger");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::warn);
}

/*static*/ void Logger::setupConsoleDebugLogging(std::string_view pattern) {
  globalLogger = spdlog::stdout_color_mt<spdlog::async_factory>("consoleDebugLogger");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::debug);
}