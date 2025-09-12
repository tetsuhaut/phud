#include "gui/TableService.hpp"

#include "db/Database.hpp"
#include "entities/Seat.hpp"
#include "filesystem/FileWatcher.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "mainLib/ProgramInfos.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp" // Future

#include <stlab/concurrency/utility.hpp>
#include <chrono>
#include <spdlog/fmt/fmt.h>
#include <filesystem>

namespace fs = std::filesystem;

static Logger LOG { CURRENT_FILE_NAME };
static constexpr std::chrono::milliseconds RELOAD_PERIOD { 2000 };

struct [[nodiscard]] TableService::Implementation final {
  Database& m_database;
  std::unique_ptr<FileWatcher> m_fileWatcher {};
  std::shared_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_reloadTask {};
  fs::path m_historyDir {};

  explicit Implementation(Database& database)
    : m_database { database } {}

  [[nodiscard]] static inline TableStatistics extractTableStatistics(auto& self,
      std::string_view table) {
    if (auto stats { self.m_database.readTableStatistics({.site = "Winamax", .table = table}) };
        stats.isValid()) {
      LOG.debug<"Got stats from db.">();
      return stats;
    } else {
      LOG.debug<"Got no stats from db yet for table {} on site {}.">(table, "Winamax");
      return TableStatistics();
    }
  }

  static void watchHistoFile(TableService::Implementation& self, const fs::path& file,
                            std::string table, auto observer) {
    self.m_fileWatcher = std::make_unique<FileWatcher>(::RELOAD_PERIOD, file);
    self.m_fileWatcher->start([&self, table, observer](const fs::path & f) {
      self.m_reloadTask = ThreadPool::submit([&self, table, observer, f]() {
        LOG.debug<"Notified, reloading the file\n{}">(f.string());
        return self.m_pokerSiteHistory->reloadFile(f);
      })
      .then([&self](const auto & pSite) { self.m_database.save(*pSite); })
      .then([&self, table]() { return extractTableStatistics(self, table); })
      .then([&self, observer](TableStatistics&& ts) { notify(std::move(ts), observer); });
    });
  }

};

static inline void notify(TableStatistics&& stats, auto statObserver) {
  if (Seat::seatUnknown == stats.getMaxSeat()) {
    LOG.debug<"Got no stats from db.">();
  } else {
    LOG.debug<"Got {} player stats objects.">(tableSeat::toInt(stats.getMaxSeat()));
    statObserver(std::move(stats));
  }
}

TableService::TableService(Database& database)
  : m_pImpl { std::make_unique<Implementation>(database) } {}

TableService::~TableService() {
  try {
    stopProducingStats();
  } catch (...) {
    LOG.error<"Unknown Error when stopping the stat production in the TableService destruction.">();
  }
}

bool TableService::isPokerApp(std::string_view executableName) const {
  const auto& exe { fs::path(executableName).filename().string() };
  const auto& stems { ProgramInfos::POKER_SITE_EXECUTABLE_STEMS };
  return std::end(stems) != std::find_if(std::begin(stems), std::end(stems),
    [&exe](const auto stem) noexcept { return exe.starts_with(stem); });
}

std::string TableService::startProducingStats(std::string_view tableWindowTitle,
                                             std::function<void(TableStatistics&&)> statObserver) {
  if (!m_pImpl->m_pokerSiteHistory) {
    return "No poker site history available";
  }

  const auto& h { m_pImpl->m_pokerSiteHistory->getHistoryFileFromTableWindowTitle(m_pImpl->m_historyDir, tableWindowTitle) };

  if (h.empty()) { 
    return fmt::format("Couldn't get history file for table '{}'", tableWindowTitle); 
  }

  const auto tableName { m_pImpl->m_pokerSiteHistory->getTableNameFromTableWindowTitle(tableWindowTitle) };
  TableService::Implementation::watchHistoFile(*m_pImpl, h, std::string(tableName), statObserver);
  return "";
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

void TableService::setPokerSiteHistory(std::shared_ptr<PokerSiteHistory> pokerSiteHistory) {
  m_pImpl->m_pokerSiteHistory = std::move(pokerSiteHistory);
}

void TableService::setHistoryDir(const fs::path& historyDir) {
  m_pImpl->m_historyDir = historyDir;
}