#pragma once

#include "filesystem/Filesystem.hpp"
#include "system/memory.hpp" // uptr
#include <chrono>
#include <functional> // std::function 

class [[nodiscard]] FileWatcher final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  FileWatcher(std::chrono::milliseconds reloadPeriod, const phud::filesystem::Path& file);
  FileWatcher(int, auto) = delete; // use only Path

  FileWatcher(const FileWatcher&) = delete;
  FileWatcher(FileWatcher&&) = delete;
  FileWatcher& operator=(const FileWatcher&) = delete;
  FileWatcher& operator=(FileWatcher&&) = delete;
  ~FileWatcher();
  /**
   * Watches periodically the previously provided file. Each time it changes,
   * @fileHasChangedCb is called.
   * Call stop() or destroy to stop the watching thread.
   * @param fileHasChangedCb called each time the file changes
   */
  void start(std::function<void(const phud::filesystem::Path&)> fileHasChangedCb);

  void stop();
  [[nodiscard]] bool isStopped() const noexcept;
}; // class FileWatcher