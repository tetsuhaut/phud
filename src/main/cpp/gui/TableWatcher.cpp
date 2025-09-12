#include "gui/TableWatcher.hpp"
#include "gui/WindowUtils.hpp"
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "threads/PeriodicTask.hpp"

#include <algorithm>
#include <chrono>
#include <ranges>
#include <vector>

static Logger LOG { CURRENT_FILE_NAME };

namespace {
  constexpr std::chrono::milliseconds WATCH_INTERVAL { 3000 }; // 3 seconds
  constexpr std::string_view WINAMAX_TABLE_PATTERN { "Winamax" };
}

struct [[nodiscard]] TableWatcher::Implementation final {
  TableWatcher::Callbacks m_callbacks;
  PeriodicTask m_periodicTask { WATCH_INTERVAL };
  std::vector<std::string> m_currentTableNames;
  bool m_hasActiveTable { false };

  explicit Implementation(const TableWatcher::Callbacks& callbacks)
    : m_callbacks { callbacks } {
    phudAssert(nullptr != m_callbacks.onTablesChanged, "m_callbacks.onTablesChanged is null");
  } 

  [[nodiscard]] static bool isPokerTable(std::string_view title) {
  // the window title should be something like 'Winamax someName someOptionalNumber'
  // e.g. 'Winamax Aalen 27', 'Winamax Athens'
  const auto nbSpaces { std::count(title.begin(), title.end(), ' ') };
    return 
      title.starts_with(WINAMAX_TABLE_PATTERN) and
      (title.length() > WINAMAX_TABLE_PATTERN.length() + 3) and
      (' ' == title.at(WINAMAX_TABLE_PATTERN.length())) and
      (' ' != title.at(WINAMAX_TABLE_PATTERN.length() + 1)) and
      ((1 == nbSpaces) or (2 == nbSpaces));
  }

  [[nodiscard]] static std::vector<std::string> findPokerTables() {
    return getWindowTitles()
      | std::views::filter(isPokerTable)
      | std::ranges::to<std::vector<std::string>>();
  }

  void checkForTables() {
    const auto& foundTables { findPokerTables() };
    
    if (!foundTables.empty()) {
      const bool hadTables { m_hasActiveTable };
      const auto& previousTables { m_currentTableNames };
      
      // Check if tables changed BEFORE updating m_currentTableNames
      if (!hadTables || previousTables != foundTables) {
        LOG.info<"Poker tables changed. Found {} table(s)">(foundTables.size());
        
        // Update state after comparison
        m_currentTableNames = foundTables;
        m_hasActiveTable = true;
        m_callbacks.onTablesChanged(foundTables);
      }
    } else {
      // No table found
      if (m_hasActiveTable) {
        LOG.info<"All poker tables lost">();
        m_hasActiveTable = false;
        m_currentTableNames.clear();
        m_callbacks.onTablesChanged({});
      }
    }
  }
}; // struct TableWatcher::Implementation

TableWatcher::TableWatcher(const Callbacks& callbacks)
  : m_pImpl { std::make_unique<Implementation>(callbacks) } {}

TableWatcher::~TableWatcher() {
  stop();
}

void TableWatcher::start() {
  LOG.debug<"Starting table watcher">();
  m_pImpl->m_periodicTask.start([this]() {
    m_pImpl->checkForTables();
    return PeriodicTaskStatus::repeatTask; // Continue watching
  });
}

void TableWatcher::stop() {
  LOG.debug<"Stopping table watcher">();
  m_pImpl->m_periodicTask.stop();
}

bool TableWatcher::isWatching() const noexcept {
  return m_pImpl->m_periodicTask.isRunning();
}

bool TableWatcher::hasActiveTable() const noexcept {
  return m_pImpl->m_hasActiveTable;
}

std::vector<std::string> TableWatcher::getCurrentTableNames() const {
  return m_pImpl->m_currentTableNames;
}

std::size_t TableWatcher::getTableCount() const noexcept {
  return m_pImpl->m_currentTableNames.size();
}