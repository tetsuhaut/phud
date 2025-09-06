#include "gui/TableWatcher.hpp"
#include "log/Logger.hpp"
#include "threads/PeriodicTask.hpp"

#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

static Logger LOG { CURRENT_FILE_NAME };

namespace {
  constexpr std::chrono::milliseconds WATCH_INTERVAL { 3000 }; // 3 seconds
  constexpr std::string_view WINAMAX_TABLE_PATTERN { "Winamax" };
}

struct [[nodiscard]] TableWatcher::Implementation final {
  TableWatcher::Callbacks m_callbacks;
  PeriodicTask m_periodicTask { WATCH_INTERVAL };
  std::string m_currentTableName;
  bool m_hasActiveTable { false };

  explicit Implementation(const TableWatcher::Callbacks& callbacks)
    : m_callbacks { callbacks } {}

  [[nodiscard]] static std::vector<std::string> getWindowTitles() {
    std::vector<std::string> titlesToBeReturned;
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
      auto& titles = *reinterpret_cast<std::vector<std::string>*>(lParam);
      
      // Check if window is visible
      if (!IsWindowVisible(hwnd)) {
        return TRUE; // Continue enumeration
      }
      
      // Get window title
      constexpr int MAX_TITLE_LENGTH = 256;
      char title[MAX_TITLE_LENGTH];
      int length = GetWindowText(hwnd, title, MAX_TITLE_LENGTH);
      
      if (length > 0) {
        titles.emplace_back(title);
      }
      
      return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&titlesToBeReturned));
    
    return titlesToBeReturned;
  }

  [[nodiscard]] static bool isWinamaxTable(std::string_view title) {
    return title.find(WINAMAX_TABLE_PATTERN) != std::string::npos;
  }

  [[nodiscard]] static std::string findWinamaxTable() {
    const auto titles = getWindowTitles();
    const auto it = std::find_if(titles.begin(), titles.end(), isWinamaxTable);
    return (it != titles.end()) ? *it : std::string{};
  }

  void checkForTables() {
    const auto foundTable = findWinamaxTable();
    
    if (!foundTable.empty()) {
      // Table found
      if (!m_hasActiveTable or foundTable != m_currentTableName) {
        LOG.info<"Winamax table found: {}">(foundTable);
        m_currentTableName = foundTable;
        m_hasActiveTable = true;
        
        if (m_callbacks.onTableFound) {
          m_callbacks.onTableFound(foundTable);
        }
      }
    } else {
      // No table found
      if (m_hasActiveTable) {
        LOG.info<"Winamax table lost: {}">(m_currentTableName);
        m_hasActiveTable = false;
        m_currentTableName.clear();
        
        if (m_callbacks.onTableLost) {
          m_callbacks.onTableLost();
        }
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

std::string TableWatcher::getCurrentTableName() const {
  return m_pImpl->m_currentTableName;
}