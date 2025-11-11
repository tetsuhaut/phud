#pragma once

#include <chrono>
#include <filesystem>
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
   * Watches periodically the file provided in the constructor. Each time it changes,
   * calls the fileHasChangedCb callback.
   * Call stop() or destroy to stop the watching thread.
   * @param fileHasChangedCb called each time the file changes
   */
  void start(const std::function<void(const std::filesystem::path&)>& fileHasChangedCb) const;
  void stop() const;
  [[nodiscard]] bool isStopped() const noexcept;
}; // class FileWatcher
