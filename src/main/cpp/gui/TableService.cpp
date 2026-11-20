#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "entities/Seat.hpp"
#include "gui/TableService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include <algorithm> // std::ranges::find_if
#include <filesystem>
#include <format>
#include <mutex>
#include <optional>
#include <ranges>
#include <unordered_map>

namespace fs = std::filesystem;

static constexpr Logger LOG {CURRENT_FILE_NAME};

namespace {
  /**
   * A table followed by the HUD.
   */
  struct [[nodiscard]] FollowedTable final {
    std::string tableName; // as written in the history file names
    TableService::TableObserverCallback observerCb;
  };

  /**
   * @returns true if the given history file name is the one of the given table: Winamax names it
   * like '20261001_Paris 10_real_holdem_no-limit.txt'.
   */
  [[nodiscard]] bool isHistoryFileOf(std::string_view fileName, const FollowedTable& table) {
    return fileName.contains(std::format("_{}_", table.tableName));
  }

  [[nodiscard]] std::optional<TableStatistics> extractTableStatistics(const Database& database,
                                                                      std::string_view table) {
    LOG.info<"Extracting table statistics for table: {}">(table);

    if (auto stats {database.readTableStatistics(ProgramInfos::WINAMAX_SITE_NAME, table)};
        stats.isValid()) {
      return stats;
    }

    LOG.info<"Got no stats from db yet for table {}.">(table);
    return {};
  }

  void notify(TableStatistics&& stats, const TableService::TableObserverCallback& observerCb) {
    if (Seat::seatUnknown == stats.getMaxSeat()) {
      LOG.info<"Got no stats from db.">();
    } else {
      LOG.info<"Got {} player stats objects.">(tableSeat::toInt(stats.getMaxSeat()));
      observerCb(std::move(stats));
    }
  }

  /**
   * Notifies the observer of the given table with its statistics, if the database has some.
   */
  void notifyIfStatsAvailable(const Database& database, const FollowedTable& table) {
    if (auto oStats = extractTableStatistics(database, table.tableName)) {
      notify(std::move(*oStats), table.observerCb);
    }
  }
} // anonymous namespace

struct [[nodiscard]] TableService::Implementation final {
  Database& m_database;
  std::shared_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  // held while the followed tables are read or changed, and while the observers are notified
  std::mutex m_mutex {};
  std::unordered_map<std::string, FollowedTable> m_tables {}; // key: the table window title

  explicit Implementation(Database& database)
    : m_database {database} {}
}; // struct TableService::Implementation

TableService::TableService(Database& database)
  : m_pImpl {std::make_unique<Implementation>(database)} {}

TableService::~TableService() {
  try {
    TableService::stopProducingStats();
  } catch (...) {
    LOG.error<"Unknown Error when stopping the stat production in the TableService destruction.">();
  }
}

/*static*/
bool TableService::isPokerApp(std::string_view executableName) {
  const auto exe = fs::path(executableName).filename().string();
  const auto stems = ProgramInfos::POKER_SITE_EXECUTABLE_STEMS;
  return std::end(stems) != std::ranges::find_if(stems, [&exe](const auto stem) noexcept {
           return exe.starts_with(stem);
         });
}

std::string TableService::startProducingStats(std::string_view tableWindowTitle,
                                              const TableObserverCallback& observerCb) {
  LOG.info<"Starting to produce stats for table window with title: '{}'">(tableWindowTitle);

  if (!m_pImpl->m_pokerSiteHistory) {
    LOG.warn<"No poker site history available">();
    return "No poker site history available";
  }

  const auto tableName =
      m_pImpl->m_pokerSiteHistory->getTableNameFromTableWindowTitle(tableWindowTitle);
  const std::scoped_lock lock {m_pImpl->m_mutex};
  // ignore if the insertion happened or not
  const auto& [it, _] {m_pImpl->m_tables.insert_or_assign(
      std::string(tableWindowTitle),
      FollowedTable {.tableName = std::string(tableName), .observerCb = observerCb})};
  // the statistics already known are shown at once, without waiting for the next hand
  notifyIfStatsAvailable(m_pImpl->m_database, it->second);
  return "";
}

void TableService::stopProducingStats() {
  const std::scoped_lock lock {m_pImpl->m_mutex};
  m_pImpl->m_tables.clear();
}

void TableService::stopProducingStatsForTable(std::string_view tableWindowTitle) {
  const std::scoped_lock lock {m_pImpl->m_mutex};
  m_pImpl->m_tables.erase(std::string(tableWindowTitle));
}

void TableService::onHistoryFileSaved(const fs::path& file) {
  const auto fileName = file.filename().string();
  const auto isTableOfThisFile = [&fileName](const auto& table) {
    return isHistoryFileOf(fileName, table);
  };
  const std::scoped_lock lock {m_pImpl->m_mutex};

  for (const auto& table :
       m_pImpl->m_tables | std::views::values | std::views::filter(isTableOfThisFile)) {
    notifyIfStatsAvailable(m_pImpl->m_database, table);
  }
}

void TableService::setPokerSiteHistory(
    std::shared_ptr<PokerSiteHistory> pokerSiteHistory) const noexcept {
  m_pImpl->m_pokerSiteHistory = std::move(pokerSiteHistory);
}
