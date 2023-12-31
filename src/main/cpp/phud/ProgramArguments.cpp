#include "filesystem/FileUtils.hpp" // std::filesystem::path, std::span
#include "phud/ProgramArguments.hpp"  // ProgramArguments::*, std::string, std::string_view, std::pair, std::optional, toLoggingLevel()
#include "mainLib/ProgramInfos.hpp"  // ProgramInfos::*
#include "strings/StringLiteral.hpp"
#include "strings/StringUtils.hpp" // phud::strings::*

#include <gsl/gsl>
#include <spdlog/formatter.h> // fmt::format

#include <array>

namespace fs = std::filesystem;
namespace ps = phud::strings;

/**
* @returns the value, from @param arguments, corresponding to one of the given options.
*/
[[nodiscard]] constexpr static std::optional<std::string_view> getOptionValue(
  std::span<const char* const>
  arguments, std::string_view shortOption, std::string_view longOption) {
  const auto begin { std::begin(arguments) }, end { std::end(arguments) };

  if (const auto it { std::find(begin, end, shortOption) }; end != it) { return gsl::at(arguments, it - begin + 1); }

  if (const auto it { std::find(begin, end, longOption) }; end != it) { return gsl::at(arguments, it - begin + 1); }

  return {};
}

[[nodiscard]] static inline std::optional<fs::path> parseHistoryDir(std::span<const char* const>
    arguments) {
  if (const auto & oDir { getOptionValue(arguments, "-d", "--historyDir") }; oDir.has_value()) {
    if (const fs::path p { oDir.value() }; phud::filesystem::isDir(p)) { return p; }

    throw ProgramArgumentsException { fmt::format("The directory '{}' does not exist.", oDir.value()) };
  }

  return {};
}

[[nodiscard]] static inline std::string toLowerCase(std::string_view str) {
  std::string lowerCase;
  lowerCase.reserve(str.size());
  std::transform(str.cbegin(), str.cend(), lowerCase.begin(), [](unsigned char c) {
    return gsl::narrow_cast<char>(std::tolower(c));
  });
  return lowerCase;
}

[[nodiscard]] static inline std::optional<LoggingLevel> parseLoggingLevel(
  std::span<const char* const> arguments) {
  if (const auto & oLogLevel { getOptionValue(arguments, "-l", "--logLevel") };
      oLogLevel.has_value()) {
    return toLoggingLevel(toLowerCase(oLogLevel.value()));
  }

  return {};
}

[[nodiscard]] static bool isOdd(auto value) { return value % 2 != 0; }

[[nodiscard]] static inline std::vector<std::string_view> listUnknownArguments(
  std::span<const char* const>
  arguments) {
  std::vector<std::string_view> ret;
  constexpr std::array<std::string_view, 4> KNOWN_ARGS {"-d", "--historyDir", "-l", "--logLevel"};
  auto index { 0 };
  std::copy_if(arguments.begin(), arguments.end(), std::back_inserter(ret),
  [&KNOWN_ARGS, &index](std::string_view arg) {
    return isOdd(index++) and
           std::end(KNOWN_ARGS) == std::find(std::begin(KNOWN_ARGS), std::end(KNOWN_ARGS), arg);
  });
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
/*[[nodiscard]]*/ std::pair<std::optional<fs::path>, std::optional<LoggingLevel>>
parseProgramArguments(std::span<const char* const> arguments) {
  const auto programName { gsl::at(arguments, 0) };
  constexpr auto USAGE_TEMPLATE { "Usage:\n{} [-d|--historyDir <directory>] "
                                  "[-l|--logLevel none|trace|info|warning|error]\n"
                                  "Where:\n"
                                  "  <directory> is the directory containing the poker site hand history.\n"
                                  "  <none|trace|info|warning|error> are the different values for the logging level.\n" };

  if ((std::end(arguments) != std::find_if(std::begin(arguments), std::end(arguments),
       isEqualTo<"-h">)) or
      (std::end(arguments) != std::find_if(std::begin(arguments), std::end(arguments),
          isEqualTo<"--help">))) {
    const auto& PROGRAM_DESCRIPTION { fmt::format("Poker Heads-Up Dispay version {} \n"
                                      "Shows statistics on the players for the current poker table.\n", ProgramInfos::APP_VERSION) };
    throw UserAskedForHelpException { fmt::format("{}{}", PROGRAM_DESCRIPTION,
                                      fmt::format(USAGE_TEMPLATE, programName)) };
  }

  if (const auto & badArgs { listUnknownArguments(arguments) }; !badArgs.empty()) {
    std::string argsList;
    std::ranges::for_each(badArgs, [&argsList](const auto & arg) { argsList.append(arg).append(", "); });
    argsList = argsList.substr(0, argsList.size() - ps::length(", "));
    throw ProgramArgumentsException { fmt::format("Unknown argument{}: {}\n{}",
                                      ps::plural(badArgs.size()), argsList, fmt::format(USAGE_TEMPLATE, programName)) };
  }

  return { parseHistoryDir(arguments), parseLoggingLevel(arguments)};
}
