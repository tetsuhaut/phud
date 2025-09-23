#pragma once

#include "language/PhudException.hpp"
#include <filesystem>
#include <optional>

// forward declarations
enum class LoggingLevel : short;

/**
 * @brief Configuration reader for PHUD settings
 */
namespace ConfigReader {
  struct Config {
    std::optional<LoggingLevel> loggingLevel = {};
    std::optional<std::filesystem::path> historyDirectory = {};
  };

  /**
   * @brief Load configuration from phud-config.properties file located next to executable
   * @param executablePath Path to the executable
   * @return Configuration structure with loaded values
   * @throw ConfigReaderException if there is an error while reading the configuration file
   */
  [[nodiscard]] Config loadConfig(const std::filesystem::path& executablePath);
} // namespace ConfigReader

class [[nodiscard]] ConfigReaderException final : public PhudException {
public:
  using PhudException::PhudException;
};
