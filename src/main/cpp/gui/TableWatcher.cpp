#include "gui/TableWatcher.hpp"
#include "gui/WindowUtils.hpp"
#include "language/Validator.hpp"
#include "log/Logger.hpp"
#include "threads/PeriodicTask.hpp"
#include <ranges>
#include <utility>
#include <vector>

static Logger& LOG() {
  static Logger logger { CURRENT_FILE_NAME };
  return logger;
}

namespace {
  constexpr std::chrono::milliseconds WATCH_INTERVAL { 3000 }; // 3 seconds
  constexpr std::string_view WINAMAX_TABLE_PATTERN { "Winamax" };

  bool isPokerTable(std::string_view title) {
    // the window title should be something like 'Winamax someName someOptionalNumber'
    // e.g. 'Winamax Aalen 27', 'Winamax Athens'
    const auto nbSpaces { std::ranges::count(title, ' ') };
    return
      title.starts_with(WINAMAX_TABLE_PATTERN) and
      (title.length() > WINAMAX_TABLE_PATTERN.length() + 3) and
      (' ' == title.at(WINAMAX_TABLE_PATTERN.length())) and
      (' ' != title.at(WINAMAX_TABLE_PATTERN.length() + 1)) and
      ((1 == nbSpaces) or (2 == nbSpaces));
  }
} // anonymous namespace

struct [[nodiscard]] TableWatcher::Implementation final {
  TableWindowsDetectedCallback m_onTablesChangedCb;
  PeriodicTask m_periodicTask { WATCH_INTERVAL };
  std::vector<std::string> m_currentTableWindowTitles {};

  explicit Implementation(TableWindowsDetectedCallback onTablesChanged)
    : m_onTablesChangedCb { std::move(onTablesChanged) } {
    validation::requireNotNull(m_onTablesChangedCb, "m_callbacks.onTablesChanged is null");
  }

  void checkForTables() {
    if (const auto& titles { mswindows::getWindowTitles()
                            | std::views::filter(::isPokerTable)
                            | std::ranges::to<std::vector<std::string>>() }; !titles.empty()) {
      if (m_currentTableWindowTitles != titles) {
        LOG().info<"Poker tables changed. Found {} table(s)">(titles.size());
        m_currentTableWindowTitles = titles;
        m_onTablesChangedCb(titles);
      }
    }
    else {
      // No table found
      if (!m_currentTableWindowTitles.empty()) {
        LOG().info<"All poker table windows lost">();
        m_currentTableWindowTitles.clear();
        m_onTablesChangedCb({});
      }
    }
  }
}; // struct TableWatcher::Implementation

TableWatcher::TableWatcher(const TableWindowsDetectedCallback& onTablesChanged)
  : m_pImpl { std::make_unique<Implementation>(onTablesChanged) } {}

TableWatcher::~TableWatcher() {
  stop();
}

void TableWatcher::start() const {
  LOG().debug<"Starting table watcher">();
  m_pImpl->m_periodicTask.start([this]() {
    m_pImpl->checkForTables();
    return PeriodicTaskStatus::repeatTask; // Continue watching
  });
}

void TableWatcher::stop() const {
  LOG().debug<"Stopping table watcher">();
  m_pImpl->m_periodicTask.stop();
}

bool TableWatcher::isWatching() const noexcept {
  return m_pImpl->m_periodicTask.isRunning();
}

std::vector<std::string> TableWatcher::getCurrentTableNames() const {
  return m_pImpl->m_currentTableWindowTitles;
}

std::size_t TableWatcher::getTableCount() const noexcept {
  return m_pImpl->m_currentTableWindowTitles.size();
}
