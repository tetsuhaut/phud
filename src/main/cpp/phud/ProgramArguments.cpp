#include "containers/algorithms.hpp" // phud::algorithms::*
#include "phud/ProgramArguments.hpp"  // ProgramArguments::*, String, StringView, Path, Pair, std::optional, toLoggingLevel()
#include "mainLib/ProgramInfos.hpp"  // ProgramInfos::*
#include <array>
#include <gsl/gsl>
#include <spdlog/fmt/bundled/format.h> // fmt::format

namespace pa = phud::algorithms;
namespace ps = phud::strings;

/**
* @returns the value, from @param arguments, corresponding to one of the given options.
*/
[[nodiscard]] constexpr static std::optional<StringView> getOptionValue(Span<const char* const>
    arguments, StringView shortOption, StringView longOption) {
  const auto begin { std::begin(arguments) }, end { std::end(arguments) };

  if (const auto it { std::find(begin, end, shortOption) }; end != it) { return gsl::at(arguments, it - begin + 1); }

  if (const auto it { std::find(begin, end, longOption) }; end != it) { return gsl::at(arguments, it - begin + 1); }

  return {};
}

[[nodiscard]] static inline std::optional<Path> parseHistoryDir(Span<const char* const>
    arguments) {
  if (const auto & oDir { getOptionValue(arguments, "-d", "--historyDir") }; oDir.has_value()) {
    const Path p { oDir.value() };

    if (!phud::filesystem::isDir(p)) {
      throw ProgramArgumentsException { fmt::format("The directory '{}' does not exist.", oDir.value()) };
    }

    return p;
  }

  return {};
}

[[nodiscard]] static inline String toLowerCase(StringView str) {
  String lowerCase;
  lowerCase.reserve(str.size());
  pa::transform(str, lowerCase, [](unsigned char c) {
    return gsl::narrow_cast<char>(std::tolower(c));
  });
  return lowerCase;
}

[[nodiscard]] static inline std::optional<LoggingLevel> parseLoggingLevel(
  Span<const char* const> arguments) {
  if (const auto & oLogLevel { getOptionValue(arguments, "-l", "--logLevel") };
      oLogLevel.has_value()) {
    return toLoggingLevel(toLowerCase(oLogLevel.value()));
  }

  return {};
}

[[nodiscard]] static bool isOdd(auto value) { return value % 2 != 0; }

[[nodiscard]] static inline Vector<StringView> listUnknownArguments(Span<const char* const>
    arguments) {
  Vector<StringView> ret;
  constexpr std::array<StringView, 4> KNOWN_ARGS {"-d", "--historyDir", "-l", "--logLevel"};
  auto index { 0 };
  std::copy_if(arguments.begin(), arguments.end(), std::back_inserter(ret),
  [&KNOWN_ARGS, &index](StringView arg) { return isOdd(index++) and !pa::contains(KNOWN_ARGS, arg); });
  return ret;
}

template<StringLiteral STR>
[[nodiscard]] static bool isEqualTo(const char* const str) noexcept {
  return 0 == std::strcmp(str, &STR.value[0]);
}

/**
 * The phud.exe program takes arguments. This function handles these arguments.
 * - No arguments passed in: typical usage, launches the graphical user interface.
 * - Any number of arguments passed in, one of which is -h or --help: shows a help message.
 * - Else, understands -d or --winamaxDir <dir>: import the history into the database.
 *                     -l or --log_level <log_level> : defines the logging level.
 * @returns the hand history directory, and the logging level
 */
/*[[nodiscard]]*/ Pair<std::optional<Path>, std::optional<LoggingLevel>>
parseProgramArguments(Span<const char* const> arguments) {
  const auto programName { gsl::at(arguments, 0) };
  constexpr auto USAGE_TEMPLATE { "Usage:\n{} [-d|--historyDir <directory>] "
                                  "[-l|--logLevel none|trace|info|warning|error]\n"
                                  "Where:\n"
                                  "  <directory> is the directory containing the poker site hand history.\n"
                                  "  <none|trace|info|warning|error> are the different values for the logging level.\n" };

  if (pa::containsIf(arguments, isEqualTo<"-h">) or pa::containsIf(arguments, isEqualTo<"--help">)) {
    const auto& PROGRAM_DESCRIPTION { fmt::format("Poker Heads-Up Dispay version {} \n"
                                      "Shows statistics on the players for the current poker table.\n", ProgramInfos::APP_VERSION) };
    throw UserAskedForHelpException { fmt::format("{}{}", PROGRAM_DESCRIPTION,
                                      fmt::format(USAGE_TEMPLATE, programName)) };
  }

  if (const auto & badArgs { listUnknownArguments(arguments) }; !badArgs.empty()) {
    String argsList;
    pa::forEach(badArgs, [&argsList](const auto & arg) { argsList.append(arg).append(", "); });
    argsList = argsList.substr(0, argsList.size() - ps::length(", "));
    throw ProgramArgumentsException { fmt::format("Unknown argument{}: {}\n{}",
                                      ps::plural(badArgs.size()), argsList, fmt::format(USAGE_TEMPLATE, programName)) };
  }

  return { parseHistoryDir(arguments), parseLoggingLevel(arguments)};
}
