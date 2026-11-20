#include "log/Logger.hpp" // std::string_view, std::string
#include <frozen/unordered_map.h>
#include <spdlog/async.h>                    // spdlog::async_factory
#include <spdlog/sinks/basic_file_sink.h>    // spdlog::basic_logger_mt
#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_mt
#include <spdlog/spdlog.h>                   // spdlog::level::level_enum, spdlog::logger
#include <cassert>                           // assert
#include <memory>                            // std::shared_ptr

using LegacyLoggingLevel = spdlog::level::level_enum;

namespace {
  // use a lazy singleton to avoid the static initialization fiasco
  // intentional leak to avoid exit-time destructor warning
  std::shared_ptr<spdlog::logger>& getGlobalLogger() {
#if defined(_MSC_VER)
#  pragma warning(suppress : 26409) // intentional leak, see above
#endif // _MSC_VER
    static auto* instance {new std::shared_ptr<spdlog::logger>()};
    return *instance;
  }

  constexpr auto LEGACY_LOGGING_LEVEL_TO_LOGGING_LEVEL {
      frozen::make_unordered_map<LegacyLoggingLevel, LoggingLevel>(
          {{LegacyLoggingLevel::critical, LoggingLevel::critical},
           {LegacyLoggingLevel::debug, LoggingLevel::debug},
           {LegacyLoggingLevel::err, LoggingLevel::error},
           {LegacyLoggingLevel::info, LoggingLevel::info},
           {LegacyLoggingLevel::trace, LoggingLevel::trace},
           {LegacyLoggingLevel::warn, LoggingLevel::warn},
           {LegacyLoggingLevel::off, LoggingLevel::none}})};

  constexpr auto LOGGING_LEVEL_TO_LEGACCY_LOGGING_LEVEL {
      frozen::make_unordered_map<LoggingLevel, LegacyLoggingLevel>(
          {{LoggingLevel::critical, LegacyLoggingLevel::critical},
           {LoggingLevel::debug, LegacyLoggingLevel::debug},
           {LoggingLevel::error, LegacyLoggingLevel::err},
           {LoggingLevel::info, LegacyLoggingLevel::info},
           {LoggingLevel::trace, LegacyLoggingLevel::trace},
           {LoggingLevel::warn, LegacyLoggingLevel::warn},
           {LoggingLevel::none, LegacyLoggingLevel::off}})};
} // anonymous namespace

[[nodiscard]] static LoggingLevel toLoggingLevel(LegacyLoggingLevel l) {
  return LEGACY_LOGGING_LEVEL_TO_LOGGING_LEVEL.find(l)->second;
}

[[nodiscard]] static LegacyLoggingLevel toLegacyLoggingLevel(LoggingLevel l) {
  return LOGGING_LEVEL_TO_LEGACCY_LOGGING_LEVEL.find(l)->second;
}

/*[[nodiscard]] static*/ LoggingLevel Logger::getCurrentLoggingLevel() {
  const auto& globalLogger {getGlobalLogger()};
  assert(nullptr != globalLogger);
  return toLoggingLevel(globalLogger->level());
}

/*static*/ void Logger::setLoggingLevel(LoggingLevel l) noexcept {
  try {
    const auto& globalLogger {getGlobalLogger()};
    assert(nullptr != globalLogger);
    globalLogger->set_level(toLegacyLoggingLevel(l));
  } catch (...) {
    // logging must never throw
  }
}

/*static*/ void Logger::shutdownLogging() noexcept {
  try {
    const auto& globalLogger {getGlobalLogger()};
    assert(nullptr != globalLogger);
    globalLogger->set_level(LegacyLoggingLevel::off);
    spdlog::drop_all();
  } catch (...) {
    // logging must never throw
  }
}

void Logger::logMessage(LoggingLevel level, std::string_view msg) const noexcept {
  try {
    const auto& globalLogger {getGlobalLogger()};
    assert(nullptr != globalLogger);
    globalLogger->log(toLegacyLoggingLevel(level), fmt::format("[{}]: {}", m_name, msg));
    globalLogger->flush();
  } catch (...) {
    // logging must never throw: the message is lost
  }
}

//
// note: *_mt means "multithread, i.e. all those factories create thread-safe loggers.
//
/*static*/ void Logger::setupFileInfoLogging(std::string_view pattern) {
  auto& globalLogger {getGlobalLogger()};
  globalLogger = spdlog::basic_logger_mt<spdlog::async_factory>("fileInfoLogger", "log.txt");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::info);
}

/*static*/ void Logger::setupConsoleWarnLogging(std::string_view pattern) {
  auto& globalLogger {getGlobalLogger()};
  globalLogger = spdlog::stdout_color_mt<spdlog::async_factory>("consoleWarnLogger");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::warn);
}

/*static*/ void Logger::setupConsoleInfoLogging(std::string_view pattern) {
  auto& globalLogger {getGlobalLogger()};
  globalLogger = spdlog::stdout_color_mt<spdlog::async_factory>("consoleInfoLogger");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::info);
}

/*static*/ void Logger::setupConsoleDebugLogging(std::string_view pattern) {
  auto& globalLogger {getGlobalLogger()};
  globalLogger = spdlog::stdout_color_mt<spdlog::async_factory>("consoleDebugLogger");
  globalLogger->set_pattern(pattern.data());
  globalLogger->set_level(LegacyLoggingLevel::debug);
}
