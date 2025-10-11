#include "filesystem/TextFile.hpp" // TextFile
#include "log/LoggingLevel.hpp"
#include "phud/ConfigReader.hpp"
#include "strings/StringUtils.hpp" // phud::strings::trim, std::string
#include <format>
#include <fstream> // std::ofstream

namespace {
  constexpr std::string_view PROPERTIES_FILE { "phud-config.properties" };

  struct KeyValue {
    std::string key;
    std::string value;
  };

  [[nodiscard]] KeyValue readLine(std::string_view line) {
    const auto trimmedLine { phud::strings::trim(line) };

    if (const auto equalPos { trimmedLine.find('=') }; !notFound(equalPos)) {
      const std::string key { phud::strings::trim(trimmedLine.substr(0, equalPos)) };
      const std::string value { phud::strings::trim(trimmedLine.substr(equalPos + 1)) };
      return KeyValue{ .key = key, .value = value };
    }
    const auto msg { std::format("Invalid configuration line (missing '='): {}", line) };
    throw ConfigReaderException(msg);
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
      if (!file.lineIsEmpty() and !file.startsWith('#')) {
        const auto [key, value] { readLine(file.getLine()) };

        /*if (key != "logging.level" and key != "history.directory" and key != "logging.pattern") {
          LOG.warn<"Unknown configuration key: '{}'">(key);
        }
        else*/ if (!value.empty()) {
          if (key == "logging.level") {
            try {
              config.loggingLevel = toLoggingLevel(value);
            } catch (const std::exception& e) {
              const auto& msg { std::format("Invalid logging level '{}': {}", value, e.what()) };
              throw ConfigReaderException(msg);
            }
          }
          else if (key == "history.directory") {
            config.historyDirectory = std::filesystem::path(value);
          }
          else { // key == "logging.pattern"
            config.loggingPattern = value;
          }
        }
      }
    }

    return config;
  }

  /**
   * @brief Create a default configuration file with default values
   * @param configPath Path where to create the configuration file
   */
  void createDefaultConfigFile(const std::filesystem::path& configPath) {
    if (std::ofstream file(configPath); file.is_open()) {
      file << "# phud Configuration File\n";
      file << "# Generated automatically with default values\n\n";
      file << "# Logging level: trace, debug, info, warn, error, critical, off\n";
      file << "logging.level=info\n\n";
      file << "# Logging pattern (spdlog format): %Y-%m-%d for YYYY-MM-DD, %Y%m%d for YYYYMMDD\n";
      file << "# See https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags\n";
      file << "logging.pattern=[%Y%m%d %H:%M:%S.%e] [%l] [%t] %v\n\n";
      file << "# History directory (leave empty if no directory configured)\n";
      file << "history.directory=\n";
      file.close();
    } else {
      throw ConfigReaderException(std::format("Failed to create configuration file: {}", configPath.string()));
    }
  }

} // anonymous namespace

ConfigReader::Config ConfigReader::loadConfig(const std::filesystem::path& executablePath) {
  const auto& configPath { executablePath.parent_path() / PROPERTIES_FILE };

  if (!std::filesystem::exists(configPath)) {
    createDefaultConfigFile(configPath);
  }

  return parsePropertiesFile(configPath);
}
