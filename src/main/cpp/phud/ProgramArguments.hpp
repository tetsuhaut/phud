#pragma once

#include "log/LoggingLevel.hpp"
#include "language/PhudException.hpp" // PhudException
#include <optional>
#include <span>
#include <stdexcept> // std::runtime_error
#include <string_view>

/**
 * The phud.exe program takes arguments. This function handles these arguments.
 * - No arguments passed in: typical usage, launches the graphical user interface.
 * - Any number of arguments passed in, one of which is -h or --help: shows a help message.
 * - Else, understands -l or --logLevel <log_level> : defines the logging level.
 * The hand history directory is chosen in the graphical user interface.
 * @returns the logging level, if given
 */
[[nodiscard]] std::optional<LoggingLevel> parseProgramArguments(std::span<const char* const> args);

/**
 * An exception thrown by the program argument parsing.
 */
class [[nodiscard]] ProgramArgumentsException final : public std::runtime_error {
public:
  explicit ProgramArgumentsException(std::string_view msg);
  ProgramArgumentsException(const ProgramArgumentsException&) = default;
  ProgramArgumentsException(ProgramArgumentsException&&) = default;
  ProgramArgumentsException& operator=(const ProgramArgumentsException&) = default;
  ProgramArgumentsException& operator=(ProgramArgumentsException&&) = default;
  ~ProgramArgumentsException() override;
};


/**
 * The exception thrown when user asks for help.
 * When user asks for help, we don't need to show the stack trace so we
 * inherit from std::runtime_error instead of PhudException.
 */
class [[nodiscard]] UserAskedForHelpException final : public std::runtime_error {
public:
  explicit UserAskedForHelpException(std::string_view msg);
  UserAskedForHelpException(const UserAskedForHelpException&) = default;
  UserAskedForHelpException(UserAskedForHelpException&&) = default;
  UserAskedForHelpException& operator=(const UserAskedForHelpException&) = default;
  UserAskedForHelpException& operator=(UserAskedForHelpException&&) = default;
  ~UserAskedForHelpException() override;
};
