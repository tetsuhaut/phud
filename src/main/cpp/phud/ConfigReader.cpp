#include "filesystem/TextFile.hpp" // TextFile
#include "log/Logger.hpp"
#include "phud/ConfigReader.hpp"
#include "strings/StringUtils.hpp" // phud::strings::trim

#include <format>
#include <string>

static Logger LOG { CURRENT_FILE_NAME };

namespace {
  constexpr std::string_view PROPERTIES_FILE { "phud-config.properties" };
  
  struct KeyValue {
    std::string key;
    std::string value;
  };
  
  [[nodiscard]] KeyValue readLine(std::string_view line) {
    const auto trimmedLine { phud::strings::trim(line) };
    const auto equalPos { trimmedLine.find('=') };
    
    if (equalPos == std::string::npos) {
      const auto msg { std::format("Invalid configuration line (missing '='): {}", line) };
      throw ConfigReaderException(msg);
    }
    const std::string key { phud::strings::trim(trimmedLine.substr(0, equalPos)) };
    const std::string value { phud::strings::trim(trimmedLine.substr(equalPos + 1)) };
    return KeyValue{ .key = key, .value = value };
  }
  
  /**
   * @brief Parse a properties file format (key=value)
   * @param configPath Path to the configuration file
   * @return Configuration structure
   */
  [[nodiscard]] ConfigReader::Config parsePropertiesFile(const std::filesystem::path& configPath) {
    ConfigReader::Config config;

    TextFile file(configPath);

    while (file.next()) {
      // Skip empty lines and comments
      if (!file.lineIsEmpty() and !file.startsWith('#')) {
        const auto [key, value] { readLine(file.getLine()) };
        
        // Parse specific properties
        if (key == "logging.level") {
          if (!value.empty()) {
            try {
              config.loggingLevel = toLoggingLevel(value);
              LOG.debug<"Read parameter logging.level: {}">(value);
            } catch (const std::exception& e) {
              const auto& msg { std::format("Invalid logging level '{}': {}", value, e.what()) };
              throw ConfigReaderException(msg);
            }
          }
        }
        else if (key == "history.directory") {
          if (!value.empty()) {
            config.historyDirectory = std::filesystem::path(value);
            LOG.debug<"Read parameter history.directory: {}">(value);
          }
        }
        else {
          LOG.warn<"Unknown configuration key: {}">(key);
        }
      }
    }

    return config;
  }

} // anonymous namespace

ConfigReader::Config ConfigReader::loadConfig(const std::filesystem::path& executablePath) {
  const auto& configPath { executablePath.parent_path() / PROPERTIES_FILE };

  if (!std::filesystem::exists(configPath)) {
    throw ConfigReaderException(std::format("Configuration file not found: {}", configPath.string()));
  }

  LOG.info<"Loading configuration from: {}">(configPath.string());
  return parsePropertiesFile(configPath);
}
