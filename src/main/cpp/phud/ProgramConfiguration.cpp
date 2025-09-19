#include "phud/ConfigReader.hpp"
#include "phud/ProgramArguments.hpp" // std::pair, std::filesystem::path, LoggingLevel
#include "phud/ProgramConfiguration.hpp" // std::pair, std::filesystem::path, LoggingLevel

std::pair<std::optional<std::filesystem::path>, LoggingLevel> ProgramConfiguration::readConfiguration(std::span<const char* const> args) {
    const auto& executablePath { std::filesystem::path(args[0]) };
    const auto& config { ConfigReader::loadConfig(executablePath) };
    // Parse command line arguments (override config file)
    const auto& [oPokerSiteHistoryDir, oLoggingLevel] { parseProgramArguments(args) };
    LoggingLevel loggingLevel;
    if (oLoggingLevel.has_value()) {
      loggingLevel = oLoggingLevel.value();
    } else if (config.loggingLevel.has_value()) {
      loggingLevel = config.loggingLevel.value();
    } else {
      throw ProgramConfigurationException("Please provide a logging level through program arguments or properties file.");
    }
  
    // Apply history directory: command line overrides config file
    std::optional<std::filesystem::path> historyDir;
    if (oPokerSiteHistoryDir.has_value()) {
      historyDir = oPokerSiteHistoryDir.value();
    } else if (config.historyDirectory.has_value()) {
      historyDir = config.historyDirectory.value();
    }
    
    return {historyDir, loggingLevel};
}