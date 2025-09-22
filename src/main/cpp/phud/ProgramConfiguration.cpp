#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "phud/ConfigReader.hpp"
#include "phud/ProgramArguments.hpp" // std::pair, std::filesystem::path, LoggingLevel
#include "phud/ProgramConfiguration.hpp" // std::pair, std::filesystem::path, LoggingLevel

static Logger LOG { CURRENT_FILE_NAME };

namespace {
}

std::pair<std::optional<std::filesystem::path>, LoggingLevel> ProgramConfiguration::readConfiguration(std::span<const char* const> args) {
  LOG.info<"reading phud configuration">();
  // Parse command line arguments (override config file)
  const auto& [oHistoDirArg, oLoggingLevel] { parseProgramArguments(args) };
  const auto& executablePath { std::filesystem::path(args[0]) };
  const auto& config { ConfigReader::loadConfig(executablePath) };
  const auto loggingLevel { [&oLoggingLevel, &config]() {
    if (oLoggingLevel.has_value()) { return oLoggingLevel.value(); }
    if (config.loggingLevel.has_value()) { return config.loggingLevel.value(); }
    throw ProgramConfigurationException("Please provide a logging level through program arguments or properties file.");
  }() };
  const auto oHistoryDir { oHistoDirArg.has_value() ? oHistoDirArg : config.historyDirectory };
  const auto histoDirStr { oHistoryDir.has_value() ? oHistoryDir.value().string() : "<none>" };
  LOG.info<"phud configuration:\n  loggingLevel={}\n  historyDirectory={}">(toString(loggingLevel), histoDirStr);
  return {oHistoryDir, loggingLevel};
} // anonymous namespace
