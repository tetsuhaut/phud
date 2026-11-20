#pragma once

#include <chrono>
#include <filesystem> // std::filesystem::path
#include <functional> // std::function
#include <memory>     // std::shared_ptr

// forward declarations
class Database;
class PokerSiteHistory;

/**
 * Validates the history directory, import the history directory, and
 * notifies observer after each new history save.
 */
class [[nodiscard]] HistoryService /*final*/ {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  /**
   * Called, from another thread, each time a history file has been saved into the database.
   */
  using FileSavedCallback = std::function<void(const std::filesystem::path& file)>;

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
   * Imports in the background every file of <dir>/history. The files are read in parallel, and each
   * one is saved as soon as it is read, in its own transaction: stopImportingHistory() returns
   * after the file being saved, and keeps the files already saved.
   * @param dir Directory containing history files
   * @param onProgress Callback called for each file processed, saved or not
   * @param onSetNbFiles Callback called when total file count is known
   * @param onDone Callback called when import is complete, not called if the import is stopped
   */
  virtual void importHistory(const std::filesystem::path& dir,
                             const std::function<void()>& onProgress,
                             const std::function<void(std::size_t)>& onSetNbFiles,
                             const std::function<void()>& onDone);

  /**
   * Imports in the background the files of <dir>/history modified after the given time, to catch
   * up with the hands played while the history was not watched.
   * @param onFileSaved called for each saved file
   * @param onDone called when every file has been imported, not called if the import is stopped
   */
  void importFilesModifiedSince(const std::filesystem::path& dir,
                                std::chrono::system_clock::time_point since,
                                const FileSavedCallback& onFileSaved,
                                const std::function<void()>& onDone);

  /**
   * Stops the imports after the file being saved, and waits for them.
   */
  virtual void stopImportingHistory();

  /**
   * Watches <dir>/history: each history file added or modified is saved into the database, then
   * onFileSaved is called. Stops watching the previous directory, if any.
   */
  void watchHistory(const std::filesystem::path& dir, const FileSavedCallback& onFileSaved);

  void stopWatchingHistory();

  /**
   * Sets the current history directory.
   * @param dir History directory path
   */
  virtual void setHistoryDir(const std::filesystem::path& dir);

  /**
   * Gets the poker site history instance.
   * @return Shared pointer to poker site history
   */
  [[nodiscard]] std::shared_ptr<PokerSiteHistory> getPokerSiteHistory() const noexcept;
};
