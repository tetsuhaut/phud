#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

// forward declarations
class Database;
class FileWatcher;
class PokerSiteHistory;
struct TableStatistics;
template<typename T> class Future;

namespace fs = std::filesystem;

/**
 * Unified service for all table-related operations.
 * Combines table selection, monitoring, and validation concerns.
 * This is a concrete sealed class - not polymorphic.
 */
class [[nodiscard]] TableService final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;
  
public:
  explicit TableService(Database& database);
  TableService(const TableService&) = delete;
  TableService(TableService&&) = delete;
  TableService& operator=(const TableService&) = delete;
  TableService& operator=(TableService&&) = delete;
  ~TableService();

  /**
   * Checks if the given executable name represents a supported poker application.
   * @param executableName Name/path of the executable
   * @return True if it's a supported poker application
   */
  [[nodiscard]] bool isPokerApp(std::string_view executableName) const;

  /**
   * Starts producing statistics for the given table.
   * @param tableName Name of table to monitor
   * @param observer Callback for statistics updates
   * @return Error message if failed, empty string if success
   */
  [[nodiscard]] std::string startProducingStats(std::string_view tableName, 
                                                std::function<void(TableStatistics&&)> observer);
  
  /**
   * Stops producing statistics for the current table.
   */
  void stopProducingStats();

  /**
   * Sets the poker site history instance to use.
   * @param pokerSiteHistory Shared pointer to poker site history
   */
  void setPokerSiteHistory(std::shared_ptr<PokerSiteHistory> pokerSiteHistory);

  /**
   * Sets the history directory.
   * @param historyDir Path to history directory
   */
  void setHistoryDir(const fs::path& historyDir);
};