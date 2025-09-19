#pragma once

#include "log/LoggingLevel.hpp"
#include "language/PhudException.hpp" // PhudException

#include <filesystem>
#include <optional>
#include <span>
#include <utility> // std::pair

namespace ProgramConfiguration {
  [[nodiscard]] std::pair<std::optional<std::filesystem::path>, LoggingLevel> readConfiguration(std::span<const char* const> args);
} // namespace ProgramConfiguration


class [[nodiscard]] ProgramConfigurationException final : public PhudException {
public:
  using PhudException::PhudException;
};