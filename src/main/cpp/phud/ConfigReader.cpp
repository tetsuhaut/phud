#include "filesystem/TextFile.hpp" // TextFile
#include "log/LoggingLevel.hpp"
#include "phud/ConfigReader.hpp"
#include "strings/StringUtils.hpp" // phud::strings::trim, std::string
#include <format>
#include <fstream> // std::ofstream

//
// NOTE: we cannot log yet as the logging framework was not initialised
//

namespace {
  constexpr std::string_view PROPERTIES_FILE = "phud-config.properties";

  [[nodiscard]] std::pair<std::string, std::string> readKeyValueFromLine(int lineNb,
                                                                         std::string_view line) {
    const auto& trimmedLine = phud::strings::trim(line);

    if (const auto equalPos = trimmedLine.find('='); !notFound(equalPos)) {
      return {std::string(phud::strings::trim(trimmedLine.substr(0, equalPos))),
              std::string(phud::strings::trim(trimmedLine.substr(equalPos + 1)))};
    }
    throw ConfigReaderException(std::format(
        "Error at line {}: invalid configuration line (missing '='): {}", lineNb, line));
  }

  /**
   * @brief Parse a properties file format (key=value)
   * @param configPath Path to the configuration file
   * @return Configuration structure
   */
  [[nodiscard]] ConfigReader::Config parsePropertiesFile(const std::filesystem::path& configPath) {
    ConfigReader::Config config;
    TextFile file(configPath);
    auto lineNb = 0;

    while (file.next()) {
      ++lineNb;

      if (!file.lineIsEmpty() and !file.startsWith('#')) {
        if (const auto [key, value] {readKeyValueFromLine(lineNb, file.getLine())};
            !value.empty()) {
          if (key == "logging.level") {
            try {
              config.loggingLevel = toLoggingLevel(value);
            } catch (const std::exception& e) {
              throw ConfigReaderException(std::format(
                  "Error at line {}: Invalid logging level '{}': {}", lineNb, value, e.what()));
            }
          } else if (key == "history.directory") {
            config.historyDirectory = std::filesystem::path(value);
          } else if (key == "logging.pattern") {
            config.loggingPattern = value;
          } else {
            throw ConfigReaderException(
                std::format("Error at line {}: unknown configuration key: '{}'", lineNb, key));
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
      throw ConfigReaderException(
          std::format("Failed to create configuration file: {}", configPath.string()));
    }
  }

} // anonymous namespace

ConfigReader::Config ConfigReader::loadConfig(const std::filesystem::path& executablePath) {
  const auto configPath = executablePath.parent_path() / PROPERTIES_FILE;

  if (!std::filesystem::exists(configPath)) {
    createDefaultConfigFile(configPath);
  }

  return parsePropertiesFile(configPath);
}
