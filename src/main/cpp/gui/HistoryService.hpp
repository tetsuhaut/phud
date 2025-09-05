#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace fs = std::filesystem;

/**
 * Unified service for all history-related operations.
 * Combines history import, validation, and management concerns.
 */
class [[nodiscard]] HistoryService {
public:
  virtual ~HistoryService() = default;
  struct [[nodiscard]] ImportCallbacks final {
    std::function<void()> onProgress;
    std::function<void(std::size_t)> onSetNbFiles;
    std::function<void()> onDone;
  };

  /**
   * Validates if the given directory contains valid poker history.
   * @param dir Directory path to validate
   * @return True if directory contains valid history files
   */
  [[nodiscard]] virtual bool isValidHistory(const fs::path& dir) = 0;
  
  /**
   * Imports history from the given directory.
   * @param dir Directory containing history files
   * @param onProgress Callback called for each file processed
   * @param onSetNbFiles Callback called when total file count is known
   * @param onDone Callback called when import is complete
   */
  virtual void importHistory(const fs::path& dir, 
                            std::function<void()> onProgress,
                            std::function<void(std::size_t)> onSetNbFiles,
                            std::function<void()> onDone) = 0;
                            
  /**
   * Sets the current history directory.
   * @param dir History directory path
   */
  virtual void setHistoryDir(const fs::path& dir) = 0;

  /**
   * Validates if the given directory contains valid poker history.
   * @param dir Directory path to validate
   * @return True if directory contains valid history files
   */
  [[nodiscard]] bool historyDirectoryIsValid(const fs::path& dir);

  /**
   * Starts importing history from the given directory.
   * @param dir Directory containing history files
   * @param callbacks Callbacks for import progress updates
   */
  void startImport(const fs::path& dir, ImportCallbacks callbacks);

  /**
   * Gets the display name for a directory (filename only).
   * @param dir Directory path
   * @return Display name for UI
   */
  [[nodiscard]] std::string getDisplayName(const fs::path& dir);
};