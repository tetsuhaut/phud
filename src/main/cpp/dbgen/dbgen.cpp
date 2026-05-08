#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp" // std::string
#include "entities/Site.hpp"
#include "filesystem/FileUtils.hpp"     // phud::filesystem::*
#include "history/PokerSiteHistory.hpp" // std::filesystem::path
#include "log/Logger.hpp" // Logger::*
#include "language/limits.hpp"          // toSizeT
#include "strings/StringUtils.hpp"
#include <optional>
#include <print>
#include <utility> // std::pair

namespace fs = std::filesystem;
namespace ps = phud::strings;

namespace {
  struct [[nodiscard]] MyLoggingConfig final {
    MyLoggingConfig() { Logger::setupConsoleWarnLogging("%v"); }
    ~MyLoggingConfig() { Logger::shutdownLogging(); }
  }; // struct MyLoggingConfig

  [[nodiscard]] bool nonCaseSentitiveEquals(std::string_view a, std::string_view b) noexcept {
    return std::ranges::equal(a, b, [](char ca, char cb) { return ps::toLowerChar(ca) == ps::toLowerChar(cb); });
  }

  [[nodiscard]] std::optional<fs::path>
  getOptionalDbHistory(std::span<const char* const> args) {
    const auto programName = std::string_view(args[0]);
    const auto askForHelp = (1 == args.size()) or std::ranges::any_of(args, [](auto arg) {
      return nonCaseSentitiveEquals(arg, "-h") or nonCaseSentitiveEquals(arg, "--help");
    });
    if (askForHelp) {
      std::println("Generates the Poker Head Up Display database.");
      std::println("{} -d <history directory>", programName);
      return {};
    }
    if ((3 != args.size()) or ("-d" != std::string_view(args[1]))) {
      std::println(stderr, "Wrong arguments.");
      std::println(stderr, "{} -d <history directory>", programName);
      return {};
    }
    const auto historyDir = fs::path(args[2]);

    if (!phud::filesystem::isDir(historyDir)  or !PokerSiteHistory::isValidHistory(historyDir)) {
      std::println(stderr, "'{}' is not a valid history directory", historyDir.string());
      std::println(stderr, "{} -d <history directory>", programName);
      return {};
    }

    return historyDir;
  }
} // anonymous namespace

int main(int argc, const char* const argv[]) {
  std::setlocale(LC_ALL, "en_US.utf8");
  MyLoggingConfig _; // will be used in the PokerSiteHistory and Database objects

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

  const auto args = std::span(argv, argv + argc);

#ifdef __clang__
#  pragma clang diagnostic pop
#endif

  if (const auto oRet = getOptionalDbHistory(args); oRet.has_value()) {
    const auto historyDir = oRet.value();
    const auto pSite = PokerSiteHistory::load(historyDir);
    auto db = Database(ProgramInfos::DATABASE_NAME);
    db.save(*pSite);
    return 0;
  }

  return 1;
}
