#pragma once

#include "log/LoggingLevel.hpp"
#include "language/PhudException.hpp" // PhudException
#include <filesystem>
#include <optional>
#include <span>
#include <utility> // std::pair

namespace ProgramConfiguration {
struct Configuration {
  std::optional<std::filesystem::path> historyDirectory;
  LoggingLevel loggingLevel;
  std::string loggingPattern;
};

[[nodiscard]] Configuration readConfiguration(std::span<const char* const> args);
} // namespace ProgramConfiguration


class [[nodiscard]] ProgramConfigurationException final : public PhudException {
 public:
  using PhudException::PhudException;
};
