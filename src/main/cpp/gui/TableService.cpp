#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "entities/Seat.hpp"
#include "entities/Site.hpp"
#include "gui/TableService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp" // Future
#include <efsw/efsw.hpp>
#include <spdlog/fmt/fmt.h>
#include <stlab/concurrency/utility.hpp>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <optional>

namespace fs = std::filesystem;

static Logger& LOG() {
  static Logger logger { CURRENT_FILE_NAME };
  return logger;
}

/**
 * EFSW-based file watcher for a single file.
 * Monitors a specific file and triggers callback on modifications.
 */
class [[nodiscard]] EfswFileWatcher final : public efsw::FileWatchListener {
private:
  // Memory layout optimized: largest to smallest to minimize padding
  efsw::FileWatcher m_watcher;
  std::function<void(const fs::path&)> m_callback;
  fs::path m_file;
  std::string m_filename;  // Cache filename for fast comparison
  std::mutex m_callbackMutex;
  efsw::WatchID m_watchId;
  std::atomic<bool> m_stopped { true };

  void handleFileAction([[maybe_unused]] efsw::WatchID watchid,
                       [[maybe_unused]] const std::string& dir,
                       const std::string& filename,
                       efsw::Action action,
                       [[maybe_unused]] std::string oldFilename) override {
    // Fast path: filter by action and filename before any expensive operations
    if (efsw::Actions::Modified != action || filename != m_filename) {
      return;
    }

    LOG().info<"The file {} has changed, notify listener">(m_file.string());

    // Call user callback in thread-safe manner
    std::lock_guard<std::mutex> lock { m_callbackMutex };
    if (m_callback) {
      m_callback(m_file);  // Reuse cached m_file instead of reconstructing path
    }
  }

public:
  explicit EfswFileWatcher(const fs::path& file)
    : m_file { file },
      m_filename { file.filename().string() },
      m_watchId { -1 } {
    LOG().info<"will watch file {} using EFSW (event-driven)">(file.string());
  }

  EfswFileWatcher(const EfswFileWatcher&) = delete;
  EfswFileWatcher(EfswFileWatcher&&) = delete;
  EfswFileWatcher& operator=(const EfswFileWatcher&) = delete;
  EfswFileWatcher& operator=(EfswFileWatcher&&) = delete;

  void start(const std::function<void(const fs::path&)>& fileHasChangedCb) {
    std::lock_guard<std::mutex> lock { m_callbackMutex };
    m_callback = fileHasChangedCb;

    // Watch the parent directory of the file
    const auto parentDir { m_file.parent_path().string() };
    m_watchId = m_watcher.addWatch(parentDir, this, false);

    if (0 > m_watchId) {
      LOG().error<"Failed to add watch for file {}">(m_file.string());
      return;
    }

    m_watcher.watch();
    m_stopped = false;
    LOG().info<"Started watching file {} (WatchID: {})">(m_file.string(), m_watchId);
  }

  void stop() {
    if (!m_stopped.load()) {
      m_watcher.removeWatch(m_watchId);
      m_stopped.store(true);
      LOG().info<"Stopped watching file {}">(m_file.string());
    }
  }

  [[nodiscard]] bool isStopped() const noexcept {
    return m_stopped.load();
  }
};

struct [[nodiscard]] TableService::Implementation final {
  Database& m_database;
  std::unique_ptr<EfswFileWatcher> m_fileWatcher {};
  std::shared_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_reloadTask {};
  fs::path m_historyDir {};

  explicit Implementation(Database& database)
    : m_database { database } {}

  [[nodiscard]] static std::optional<TableStatistics> extractTableStatistics(const Database& database,
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

  static std::unique_ptr<EfswFileWatcher> watchHistoFile(Future<void>& reloadTask,
                                                         std::shared_ptr<PokerSiteHistory> pokerSiteHistory, // NOLINT(*-unnecessary-value-param)
                                                         Database& database,
                                                         const fs::path& file,
                                                         std::string_view aTable,
                                                         const auto& observerCb) {
    LOG().info<"Starting to watch history file: {} for table: {}">(file.string(), aTable);
    auto fileWatcher = std::make_unique<EfswFileWatcher>(file);
    fileWatcher->start(
      [&reloadTask, &database, pokerSiteHistory, table = std::string(aTable), observerCb](const fs::path& f) {
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
                     .then([observerCb](std::optional<TableStatistics>&& ots) {
                        if (ots.has_value()) {
                          LOG().info<"in threadpool : Notifying observer with table statistics">();
                          notify(std::move(ots.value()), observerCb);
                        }
                        else {
                          LOG().warn<"in threadpool : No statistics found">();
                        }
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
