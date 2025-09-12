#pragma once

#include "statistics/TableStatistics.hpp"
#include "gui/Rectangle.hpp" // phud::Rectangle

#include <functional>
#include <string>

// forward declaration
struct TableStatistics;

/**
 * Unified service for all table-related operations.
 * Combines table selection, monitoring, and validation concerns.
 */
class [[nodiscard]] TableService {
private:
  std::string m_currentTableName{};
  
public:
  virtual ~TableService() = default;

  /**
   * Checks if the given executable name represents a supported poker application.
   * @param executableName Name/path of the executable
   * @return True if it's a supported poker application
   */
  [[nodiscard]] virtual bool isPokerApp(std::string_view executableName) const = 0;

  /**
   * Starts producing statistics for the given table.
   * @param tableName Name of table to monitor
   * @param observer Callback for statistics updates
   * @return Error message if failed, empty string if success
   */
  [[nodiscard]] virtual std::string startProducingStats(std::string_view tableName, 
                                                        std::function<void(TableStatistics&&)> observer) = 0;
  
  /**
   * Stops producing statistics for the current table.
   */
  virtual void stopProducingStats() = 0;

  /**
   * Starts monitoring statistics for the given table.
   * @param tableName Name of table to monitor
   * @param observer Callback for statistics updates
   * @return Error message if failed, empty string if success
   */
  [[nodiscard]] std::string startMonitoringTable(std::string_view tableName, 
                                                 std::function<void(TableStatistics&&)> observer);

  /**
   * Stops monitoring the current table.
   */
  void stopMonitoring();
};