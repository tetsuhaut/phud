#include "filesystem/FileUtils.hpp" // std::filesystem::path, std::span
#include "phud/ProgramArguments.hpp" // ProgramArguments::*, std::string, std::string_view, std::pair, std::optional, toLoggingLevel()

#include "constants/ProgramInfos.hpp" // ProgramInfos::*
#include "strings/StringLiteral.hpp"
#include "strings/StringUtils.hpp" // phud::strings::*
#include <gsl/gsl>
#include <spdlog/fmt/bundled/format.h> // fmt::format
#include <array>
#include <format>
#include <ranges> // std::ranges, std::views

namespace fs = std::filesystem;
namespace ps = phud::strings;

namespace {
  constexpr auto USAGE_TEMPLATE =
      "Usage:\n{} [-d|--historyDir <directory>] "
      "[-l|--logLevel none|trace|info|warning|error]\n"
      "Where <directory> is the directory containing the poker site hands history.\n";

  [[nodiscard]] std::string strToLowerCase(std::string_view str) {
    std::string lowerCase;
    lowerCase.reserve(str.size());
    std::ranges::transform(str, std::back_inserter(lowerCase), ps::toLowerChar);
    return lowerCase;
  }

  [[nodiscard]] std::vector<std::string> toLowerCase(std::span<const char* const> input) {
    return input | std::views::transform(strToLowerCase) | std::ranges::to<std::vector>();
  }

  template <StringLiteral STR>
  [[nodiscard]] constexpr bool isEqualTo(std::string_view str) noexcept {
    return str == &STR.value[0];
  }
} // anonymous namespace

/**
 * @returns the value, from arguments, corresponding to one of the given options.
 * @param arguments the given arguments
 * @param shortOption like -o
 * @param longOption like --Option
 */
[[nodiscard]] constexpr static std::optional<std::string_view>
getOptionValue(std::span<const char* const> arguments, std::string_view shortOption,
               std::string_view longOption) {
  const auto begin = std::begin(arguments);
  const auto end = std::end(arguments);

  if (const auto it {std::find(begin, end, shortOption)}; end != it) {
    return gsl::at(arguments, it - begin + 1);
  }

  if (const auto it {std::find(begin, end, longOption)}; end != it) {
    return gsl::at(arguments, it - begin + 1);
  }

  return {};
}

[[nodiscard]] static std::optional<fs::path>
parseHistoryDir(std::span<const char* const> arguments) {
  if (const auto oDir = getOptionValue(arguments, "-d", "--historyDir"); oDir.has_value()) {
    if (fs::path p = oDir.value(); phud::filesystem::isDir(p)) {
      return p;
    }

    throw ProgramArgumentsException {
        fmt::format("The directory '{}' does not exist.", oDir.value())};
  }

  return {};
}

[[nodiscard]] static std::optional<LoggingLevel>
parseLoggingLevel(std::span<const char* const> arguments) {
  if (const auto oLogLevel = getOptionValue(arguments, "-l", "--logLevel"); oLogLevel.has_value()) {
    return toLoggingLevel(strToLowerCase(oLogLevel.value()));
  }

  return {};
}

[[nodiscard]] constexpr static bool isOdd(auto value) {
  return value % 2 != 0;
}

[[nodiscard]] static std::vector<std::string_view>
listUnknownArguments(std::span<const char* const> arguments) {
  std::vector<std::string_view> ret;
  constexpr std::array<std::string_view, 4> KNOWN_ARGS = {"-d", "--historyDir", "-l", "--logLevel"};
  auto index = 0;
  std::ranges::copy_if(
      arguments, std::back_inserter(ret), [&KNOWN_ARGS, &index](std::string_view arg) {
        return isOdd(index++) and (std::end(KNOWN_ARGS) == std::ranges::find(KNOWN_ARGS, arg));
      });
  return ret;
}

/**
 * The phud.exe program takes arguments. This function handles these arguments.
 * - No arguments passed in: typical usage, launches the graphical user interface.
 * - Any number of arguments passed in, one of which is -h or --help: shows a help message.
 * - Else, understands -d or --winamaxDir <dir>: import the history into the database.
 *                     -l or --log_level <log_level> : defines the logging level.
 * @returns the hand history directory, and the logging level
 * @throws UserAskedForHelpException if args contains "-h" or "--help"
 */
/*[[nodiscard]]*/
std::pair<std::optional<fs::path>, std::optional<LoggingLevel>>
parseProgramArguments(std::span<const char* const> args) {
  // NOTE: can't log yet
  const auto programName = gsl::at(args, 0);
  const auto usageMsg = fmt::format(::USAGE_TEMPLATE, programName);
  const auto progArgs = ::toLowerCase(args.subspan(1));

  if ((std::end(progArgs) != std::ranges::find_if(progArgs, ::isEqualTo<"-h">)) or
      (std::end(progArgs) != std::ranges::find_if(progArgs, ::isEqualTo<"--help">))) {
    const auto PROGRAM_DESCRIPTION =
        fmt::format("\nPoker Heads-Up Dispay version {} \n"
                    "Shows statistics on the players for the current poker table.\n",
                    ProgramInfos::APP_VERSION);
    throw UserAskedForHelpException(fmt::format("{}\n{}", PROGRAM_DESCRIPTION, usageMsg));
  }

  if (const auto badArgs = listUnknownArguments(args); !badArgs.empty()) {
    std::string argsList;
    std::ranges::for_each(badArgs,
                          [&argsList](const auto& arg) { argsList.append(arg).append(", "); });
    argsList = argsList.substr(0, argsList.size() - ps::length(", "));
    const auto errMsg = fmt::format("Unknown argument{}: {}\n{}", ps::plural(badArgs.size()), argsList, usageMsg);
    throw ProgramArgumentsException {errMsg};
  }

  return {parseHistoryDir(args), parseLoggingLevel(args)};
}

ProgramArgumentsException::ProgramArgumentsException(std::string_view msg)
  : std::exception(std::format("{}", msg).c_str()) {}

ProgramArgumentsException::~ProgramArgumentsException() = default;

UserAskedForHelpException::UserAskedForHelpException(std::string_view msg)
  : std::exception(std::format("{}", msg).c_str()) {}

UserAskedForHelpException::~UserAskedForHelpException() = default;
