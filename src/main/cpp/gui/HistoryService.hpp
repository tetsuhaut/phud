#pragma once

#include <filesystem> // std::filesystem::path
#include <functional> // std::function
#include <memory>     // std::shared_ptr

// forward declarations
class Database;
class PokerSiteHistory;

/**
 * Unified service for all history-related operations.
 * Combines history import, validation, and management concerns.
 * This is a concrete sealed class - not polymorphic.
 */
class [[nodiscard]] HistoryService /*final*/ {
 private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

 public:
  explicit HistoryService(Database& database);
  HistoryService(const HistoryService&) = delete;
  HistoryService(HistoryService&&) = delete;
  HistoryService& operator=(const HistoryService&) = delete;
  HistoryService& operator=(HistoryService&&) = delete;
  virtual ~HistoryService();

  /**
   * Validates if the given directory contains valid poker history.
   * @param dir Directory path to validate
   * @return True if directory contains valid history files
   */
  [[nodiscard]] virtual bool isValidHistory(const std::filesystem::path& dir);

  /**
   * Imports history from the given directory.
   * @param dir Directory containing history files
   * @param onProgress Callback called for each file processed
   * @param onSetNbFiles Callback called when total file count is known
   * @param onDone Callback called when import is complete
   */
  virtual void importHistory(const std::filesystem::path& dir,
                             const std::function<void()>& onProgress,
                             const std::function<void(std::size_t)>& onSetNbFiles,
                             const std::function<void()>& onDone);

  /**
   * Stops importing history.
   */
  virtual void stopImportingHistory();

  /**
   * Sets the current history directory.
   * @param dir History directory path
   */
  virtual void setHistoryDir(const std::filesystem::path& dir);

  /**
   * Gets the poker site history instance.
   * @return Shared pointer to poker site history
   */
  [[nodiscard]] std::shared_ptr<PokerSiteHistory> getPokerSiteHistory() const;
};
