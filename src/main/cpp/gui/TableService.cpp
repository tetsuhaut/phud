#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "entities/Seat.hpp"
#include "entities/Site.hpp"
#include "filesystem/FileWatcher.hpp"
#include "gui/TableService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp" // Future
#include <spdlog/fmt/fmt.h>
#include <stlab/concurrency/utility.hpp>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

static Logger& LOG() {
  static Logger logger { CURRENT_FILE_NAME };
  return logger;
}
static constexpr std::chrono::milliseconds RELOAD_PERIOD { 2000 };

struct [[nodiscard]] TableService::Implementation final {
  Database& m_database;
  std::unique_ptr<FileWatcher> m_fileWatcher {};
  std::shared_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_reloadTask {};
  fs::path m_historyDir {};

  explicit Implementation(Database& database)
    : m_database { database } {}

  [[nodiscard]] static TableStatistics extractTableStatistics(const Database& database,
                                                              std::string_view table) {
    LOG().info<"Extracting table statistics for table: {}">(table);
    if (auto stats { database.readTableStatistics("Winamax", table) };
      stats.isValid()) {
      LOG().info<"Got stats from db.">();
      return stats;
    }
    LOG().info<"Got no stats from db yet for table {} on site {}.">(table, "Winamax");
    return {};
  }

  static std::unique_ptr<FileWatcher> watchHistoFile(Future<void>& reloadTask,
                                                     std::shared_ptr<PokerSiteHistory> pokerSiteHistory, // NOLINT(*-unnecessary-value-param)
                                                     Database& database,
                                                     const fs::path& file,
                                                     std::string_view table,
                                                     const auto& observerCb) {
    LOG().info<"Starting to watch history file: {} for table: {}">(file.string(), table);
    auto fileWatcher = std::make_unique<FileWatcher>(::RELOAD_PERIOD, file);
    fileWatcher->start(
      [&reloadTask, &database, pokerSiteHistory, table = std::string(table), observerCb](const fs::path& f) {
        LOG().info<"File watcher triggered for: {}">(f.string());
        reloadTask = ThreadPool::submit([pokerSiteHistory, table, observerCb, f]() {
                       LOG().debug<"Notified, reloading the file\n{}">(f.string());
                       return pokerSiteHistory->reloadFile(f);
                     })
                     .then([&database](const auto& pSite) {
                       LOG().info<"in threadpool : Saving poker site data to database">();
                       database.save(*pSite);
                     })
                     .then([&database, table]() {
                       LOG().info<"in threadpool : Extracting table statistics for table: {}">(table);
                       return extractTableStatistics(database, table);
                     })
                     .then([observerCb](TableStatistics&& ts) {
                       LOG().info<"in threadpool : Notifying observer with table statistics">();
                       notify(std::move(ts), observerCb);
                     });
      });
    return fileWatcher;
  }
}; // struct TableService::Implementation

static void notify(TableStatistics&& stats, const auto& observerCb) {
  if (Seat::seatUnknown == stats.getMaxSeat()) {
    LOG().info<"Got no stats from db.">();
  }
  else {
    LOG().info<"Got {} player stats objects.">(tableSeat::toInt(stats.getMaxSeat()));
    LOG().info<"Calling observer with statistics...">();
    observerCb(std::move(stats));
  }
}

TableService::TableService(Database& database)
  : m_pImpl { std::make_unique<Implementation>(database) } {}

TableService::~TableService() {
  try {
    TableService::stopProducingStats();
  }
  catch (...) {
    LOG().error<"Unknown Error when stopping the stat production in the TableService destruction.">();
  }
}

/*static*/
bool TableService::isPokerApp(std::string_view executableName) {
  const auto& exe { fs::path(executableName).filename().string() };
  const auto& stems { ProgramInfos::POKER_SITE_EXECUTABLE_STEMS };
  return std::end(stems) != std::ranges::find_if(stems,
                                                 [&exe](const auto stem) noexcept { return exe.starts_with(stem); });
}

static bool wasUpdatedLessThat2MinutesAgo(const fs::path& p) noexcept {
  const auto& lastUpdateTime { std::chrono::time_point_cast<std::chrono::system_clock::duration>(
    fs::last_write_time(p) - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
  ) };
  const auto& now { std::chrono::system_clock::now() };
  const auto age { std::chrono::duration_cast<std::chrono::minutes>(now - lastUpdateTime) };
  return age < std::chrono::minutes(2);
}


std::string TableService::startProducingStats(std::string_view tableWindowTitle,
                                              const TableObserverCallback& observerCb) {
  LOG().info<"Starting to produce stats for table window with title: '{}'">(tableWindowTitle);

  if (!m_pImpl->m_pokerSiteHistory) {
    LOG().warn<"No poker site history available">();
    return "No poker site history available";
  }

  if (const auto& oHistoFile {
        m_pImpl->m_pokerSiteHistory->getHistoryFileFromTableWindowTitle(m_pImpl->m_historyDir, tableWindowTitle)
      }; oHistoFile.has_value() and wasUpdatedLessThat2MinutesAgo(oHistoFile.value())) {
    const auto& histoFile { oHistoFile.value() };
    const auto tableName { m_pImpl->m_pokerSiteHistory->getTableNameFromTableWindowTitle(tableWindowTitle) };
    LOG().info<"Table name: '{}', history file: '{}'">(tableName, histoFile.string());
    m_pImpl->m_fileWatcher = Implementation::watchHistoFile(m_pImpl->m_reloadTask, m_pImpl->m_pokerSiteHistory,
      m_pImpl->m_database, histoFile, tableName, observerCb);
    return "";
  }
  else {
    LOG().warn<"Couldn't get history file for table '{}'">(tableWindowTitle);
    return fmt::format("Couldn't get history file for table '{}'", tableWindowTitle);
  }
}

void TableService::stopProducingStats() {
  if (nullptr != m_pImpl->m_fileWatcher) {
    m_pImpl->m_fileWatcher->stop();
  }
  if (m_pImpl->m_reloadTask.valid()) {
    stlab::await(std::move(m_pImpl->m_reloadTask));
  }
  m_pImpl->m_reloadTask.reset();
}

void TableService::setPokerSiteHistory(std::shared_ptr<PokerSiteHistory> pokerSiteHistory) const {
  m_pImpl->m_pokerSiteHistory = std::move(pokerSiteHistory);
}

void TableService::setHistoryDir(const fs::path& historyDir) const {
  m_pImpl->m_historyDir = historyDir;
}
