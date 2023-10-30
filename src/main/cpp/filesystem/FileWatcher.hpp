#pragma once

#include "filesystem/Filesystem.hpp"

#include <chrono>
#include <functional> // std::function
#include <memory> // std::unique_ptr

class [[nodiscard]] FileWatcher final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  FileWatcher(std::chrono::milliseconds reloadPeriod, const std::filesystem::path& file);
  FileWatcher(int, auto) = delete; // use only std::filesystem::path

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
  void start(std::function<void(const std::filesystem::path&)> fileHasChangedCb);

  void stop();
  [[nodiscard]] bool isStopped() const noexcept;
}; // class FileWatcher