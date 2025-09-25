#include "db/Database.hpp" // std::string
#include "entities/Site.hpp"
#include "filesystem/FileUtils.hpp" // phud::filesystem::*
#include "history/PokerSiteHistory.hpp" // std::filesystem::path
#include "language/limits.hpp" // toSizeT
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include <optional>
#include <utility> // std::pair

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;

namespace {
  struct [[nodiscard]] LoggingConfig final {
    LoggingConfig() { Logger::setupConsoleWarnLogging("%v"); }
    ~LoggingConfig() { Logger::shutdownLogging(); }
  }; // struct LoggingConfig

  [[nodiscard]] std::optional<std::pair<fs::path, fs::path>> getOptionalDbAndHistory(
    std::span<const char* const> args) {
    if (5 != args.size()) {
      if ((1 == args.size())) {
        LOG.error<"{} -b <database file name> -d <history directory>\n">(args[0]);
      }
      else {
        LOG.error<"Wrong arguments.">();
        LOG.error<"{} -b <database file name> -d <history directory>\n">(args[0]);
      }

      return {};
    }

    const std::string_view flag1 { args[1] };

    if (const std::string_view flag2 { args[3] }; ("-b" != flag1 and "-d" != flag1) or ("-b" != flag2 and "-d" !=
      flag2)) {
      LOG.error<"Wrong arguments.">();
      LOG.error<"{} -b <database file name> -d <history directory>\n">(args[0]);
      return {};
    }

    const fs::path dbFile { ("-b" == flag1) ? args[2] : args[4] };
    const fs::path historyDir { ("-b" == flag1) ? args[4] : args[2] };

    if (phud::filesystem::isFile(dbFile)) {
      LOG.error<"The database file\n{}\nalready exists and is in the way.">(dbFile.string());
      return {};
    }

    if (!PokerSiteHistory::isValidHistory(historyDir)) {
      LOG.error<"'{}' is not a valid history directory">(historyDir.string());
      LOG.error<"{} -b <database file name> -d <history directory>\n">(args[0]);
      return {};
    }

    return std::make_pair(dbFile, historyDir);
  }
} // anonymous namespace

int main(int argc, const char* const argv[]) {
  std::setlocale(LC_ALL, "en_US.utf8");
  LoggingConfig _;

  if (const auto optionalRet { getOptionalDbAndHistory(std::span(argv, limits::toSizeT(argc))) };
    optionalRet.has_value()) {
    const auto [dbFile, historyDir] { optionalRet.value() };
    const auto& pSite { PokerSiteHistory::newInstance(historyDir)->load(historyDir) };
    Database db { dbFile.string() };
    db.save(*pSite);
    return 0;
  }

  return 1;
}
