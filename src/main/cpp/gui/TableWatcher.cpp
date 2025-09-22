#include "gui/TableWatcher.hpp"
#include "gui/WindowUtils.hpp"
#include "language/Validator.hpp"
#include "log/Logger.hpp"
#include "threads/PeriodicTask.hpp"

#include <ranges>
#include <vector>

static Logger LOG { CURRENT_FILE_NAME };

namespace {
  constexpr std::chrono::milliseconds WATCH_INTERVAL { 3000 }; // 3 seconds
  constexpr std::string_view WINAMAX_TABLE_PATTERN { "Winamax" };
} // anonymous namespace

struct [[nodiscard]] TableWatcher::Implementation final {
  TablesChangedCallback m_onTablesChangedCb;
  PeriodicTask m_periodicTask { WATCH_INTERVAL };
  std::vector<std::string> m_currentTableNames {};

  explicit Implementation(const TablesChangedCallback& onTablesChanged)
    : m_onTablesChangedCb { onTablesChanged } {
    validation::requireNotNull(m_onTablesChangedCb, "m_callbacks.onTablesChanged is null");
  }

  [[nodiscard]] static std::vector<std::string> findPokerTables() {
    return getWindowTitles()
      | std::views::filter(TableWatcher::isPokerTable)
      | std::ranges::to<std::vector<std::string>>();
  }

  void checkForTables() {
    if (const auto& foundTables { findPokerTables() }; !foundTables.empty()) {
      // Check if tables changed BEFORE updating m_currentTableNames
      if (m_currentTableNames != foundTables) {
        LOG.info<"Poker tables changed. Found {} table(s)">(foundTables.size());
        
        // Update state after comparison
        m_currentTableNames = foundTables;
        m_onTablesChangedCb(foundTables);
      }
    } else {
      // No table found
      if (!m_currentTableNames.empty()) {
        LOG.info<"All poker tables lost">();
        m_currentTableNames.clear();
        m_onTablesChangedCb({});
      }
    }
  }
}; // struct TableWatcher::Implementation

bool TableWatcher::isPokerTable(std::string_view title) {
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

TableWatcher::TableWatcher(const TablesChangedCallback& onTablesChanged)
  : m_pImpl { std::make_unique<Implementation>(onTablesChanged) } {}

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

std::vector<std::string> TableWatcher::getCurrentTableNames() const {
  return m_pImpl->m_currentTableNames;
}

std::size_t TableWatcher::getTableCount() const noexcept {
  return m_pImpl->m_currentTableNames.size();
}
