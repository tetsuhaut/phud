#pragma once

#include <chrono>
#include <filesystem> // std::filesystem::path
#include <functional> // std::function
#include <memory> // std::unique_ptr

class [[nodiscard]] DirWatcher final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  DirWatcher(std::chrono::milliseconds reloadPeriod, const std::filesystem::path& dir);
  DirWatcher(int, auto) = delete; // use only std::filesystem::path

  DirWatcher(const DirWatcher&) = delete;
  DirWatcher(DirWatcher&&) = delete;
  DirWatcher& operator=(const DirWatcher&) = delete;
  DirWatcher& operator=(DirWatcher&&) = delete;
  ~DirWatcher();
  /**
   * Watches periodically the previously provided directory. Each time a file
   * inside changes, @fileHasChangedCb is called.
   * Call stop() or destroy to stop the watching thread.
   * @param fileHasChangedCb called each time a file changes
   */
  void start(std::function<void(const std::filesystem::path&)> fileHasChangedCb);

  void stop();
  [[nodiscard]] bool isStopped() const noexcept;
}; // class DirWatcher