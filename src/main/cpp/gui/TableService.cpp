#include "gui/TableService.hpp"
#include "gui/WindowUtils.hpp"
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"

static Logger LOG { CURRENT_FILE_NAME };

std::string TableService::startMonitoringTable(std::string_view tableName, 
                                               std::function<void(TableStatistics&&)> observer) {
  LOG.info<"Starting statistics monitoring for table: '{}'">(tableName);
  const auto& errorMsg = startProducingStats(tableName, observer);
  
  if (errorMsg.empty()) {
    m_currentTableName = std::string(tableName);
    LOG.info<"Successfully started monitoring table: '{}'">(tableName);
  } else {
    LOG.error<"Failed to start monitoring table '{}': {}">(tableName, errorMsg);
  }
  
  return errorMsg;
}

void TableService::stopMonitoring() {
  if (!m_currentTableName.empty()) {
    LOG.info<"Stopping monitoring for table: '{}'">(m_currentTableName);
    stopProducingStats();
    m_currentTableName.clear();
  }
}