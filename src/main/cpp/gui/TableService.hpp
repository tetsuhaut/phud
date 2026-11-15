#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

// forward declarations
class Database;
class PokerSiteHistory;
struct TableStatistics;

/**
 * Produces the statistics of the tables displayed by the HUD.
 */
class [[nodiscard]] TableService /*final*/ {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  using TableObserverCallback = std::function<void(TableStatistics&&)>;

  explicit TableService(Database& database);
  TableService(const TableService&) = delete;
  TableService(TableService&&) = delete;
  TableService& operator=(const TableService&) = delete;
  TableService& operator=(TableService&&) = delete;
  virtual ~TableService();

  /**
   * Checks if the given executable name represents a supported poker application.
   * @param executableName Name/path of the executable
   * @return True if it's a supported poker application
   */
  [[nodiscard]] static bool isPokerApp(std::string_view executableName);

  /**
   * Starts producing statistics for the given table: observerCb will be called each time the
   * history file of this table is saved. Several tables can be followed at the same time.
   * @param tableWindowTitle title of the window of the table to follow
   * @param observerCb Callback for statistics updates, called from another thread
   * @return Error message if failed, empty string if success
   */
  [[nodiscard]] virtual std::string startProducingStats(std::string_view tableWindowTitle,
                                                        const TableObserverCallback& observerCb);
  /**
   * Stops producing statistics for every table. When it returns, no observer is called anymore.
   */
  virtual void stopProducingStats();

  /**
   * Stops producing statistics for the given table only.
   */
  void stopProducingStatsForTable(std::string_view tableWindowTitle);

  /**
   * Notifies the observers of the tables which history is the given file, which has just been saved
   * into the database.
   */
  void onHistoryFileSaved(const std::filesystem::path& file);

  /**
   * Sets the poker site history instance to use.
   * @param pokerSiteHistory Shared pointer to poker site history
   */
  void setPokerSiteHistory(std::shared_ptr<PokerSiteHistory> pokerSiteHistory) const;
}; // class TableService
