#pragma once

#include "log/LoggingLevel.hpp"
#include "language/PhudException.hpp" // PhudException
#include <span>
#include <string>

namespace ProgramConfiguration {
  struct Configuration {
    LoggingLevel loggingLevel;
    std::string loggingPattern;
  };

  [[nodiscard]] Configuration readConfiguration(std::span<const char* const> args);
} // namespace ProgramConfiguration


class [[nodiscard]] ProgramConfigurationException final : public PhudException {
public:
  using PhudException::PhudException;
};
