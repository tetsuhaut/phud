#pragma once

#include <chrono>
#include <filesystem> // std::filesystem::path
#include <functional> // std::function
#include <memory>     // std::unique_ptr

class [[nodiscard]] DirWatcher /*final*/ {
public:
  /**
   * @param dir The directory to watch for file changes
   * @return A unique pointer to the DirWatcher instance
   */
  [[nodiscard]] static std::unique_ptr<DirWatcher> create(const std::filesystem::path& dir);
  virtual ~DirWatcher();
  /**
   * Watches the previously provided directory using native OS file system events.
   * Each time a .txt file inside changes (Modified or Add action), the callback is called.
   * Call stop() or destroy to stop the watching.
   * @param fileHasChangedCb called each time a file changes
   */
  virtual void start(const std::function<void(const std::filesystem::path&)>& fileHasChangedCb) = 0;
  virtual void stop() = 0;
  [[nodiscard]] virtual bool isStopped() const noexcept = 0;
}; // class DirWatcher
