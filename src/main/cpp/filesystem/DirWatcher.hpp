#pragma once

#include <chrono>
#include <filesystem> // std::filesystem::path
#include <functional> // std::function
#include <memory> // std::unique_ptr

class [[nodiscard]] DirWatcher /*final*/ {
public:
  [[nodiscard]] static std::unique_ptr<DirWatcher> create(std::chrono::milliseconds reloadPeriod, const std::filesystem::path& dir);
  virtual ~DirWatcher();
  /**
   * Watches periodically the previously provided directory. Each time a file
   * inside changes, @fileHasChangedCb is called.
   * Call stop() or destroy to stop the watching thread.
   * @param fileHasChangedCb called each time a file changes
   */
  virtual void start(const std::function<void(const std::filesystem::path&)>& fileHasChangedCb) = 0;
  virtual void stop() const = 0;
  [[nodiscard]] virtual bool isStopped() const noexcept = 0;
}; // class DirWatcher
