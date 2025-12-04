#include "phud/ConfigReader.hpp"
#include "phud/ProgramArguments.hpp"     // std::pair, std::filesystem::path, LoggingLevel
#include "phud/ProgramConfiguration.hpp" // std::pair, std::filesystem::path, LoggingLevel

namespace {
constexpr auto DEFAULT_LOGGING_PATTERN {"[%Y%m%d %H:%M:%S.%e] [%l] [%t] %v"};
}

ProgramConfiguration::Configuration
ProgramConfiguration::readConfiguration(std::span<const char* const> args) {
  // Parse command line arguments (override config file)
  // NOTE: we cannot log yet as the logging framework was not created
  const auto [oHistoDirArg, oLoggingLevel] = parseProgramArguments(args);
  const auto executablePath = std::filesystem::path(args[0]);
  const auto config = ConfigReader::loadConfig(executablePath);
  const auto loggingLevel = [&oLoggingLevel, &config]() {
    if (oLoggingLevel.has_value()) {
      return oLoggingLevel.value();
    }
    if (config.loggingLevel.has_value()) {
      return config.loggingLevel.value();
    }
    throw ProgramConfigurationException(
        "Please provide a logging level through program arguments or properties file.");
  }();
  const auto loggingPattern =
      config.loggingPattern.has_value() ? config.loggingPattern.value() : DEFAULT_LOGGING_PATTERN;
  const auto oHistoryDir = oHistoDirArg.has_value() ? oHistoDirArg : config.historyDirectory;
  const auto histoDirStr = oHistoryDir.has_value() ? oHistoryDir.value().string() : "<none>";
  return Configuration {oHistoryDir, loggingLevel, loggingPattern};
} // anonymous namespace
