#pragma once

#include "log/LoggingLevel.hpp"
#include "language/PhudException.hpp" // PhudException

#include <filesystem>
#include <optional>
#include <span>
#include <utility> // std::pair

/**
 * The phud.exe program takes arguments. This function handles these arguments.
 * - No arguments passed in: typical usage, launches the graphical user interface.
 * - Any number of arguments passed in, one of which is -h or --help: shows a help message.
 * - Else, understands -d or --winamaxDir <dir>: import the history into the database.
 *                     -l or --logLevel <log_level> : defines the logging level.
 * @returns the hand history directory, and the logging level
 */
[[nodiscard]] std::pair<std::optional<std::filesystem::path>, std::optional<LoggingLevel>> parseProgramArguments(
      std::span<const char* const> args); 

class [[nodiscard]] ProgramArgumentsException : public PhudException {
public:
  using PhudException::PhudException;
};

class [[nodiscard]] UserAskedForHelpException final : public ProgramArgumentsException {
public:
  using ProgramArgumentsException::ProgramArgumentsException;
};