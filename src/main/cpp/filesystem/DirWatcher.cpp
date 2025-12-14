#include "filesystem/DirWatcher.hpp" // std::chrono, std::filesystem::path, std::string
#include "filesystem/FileUtils.hpp"  // phud::filesystem::*
#include "language/Validator.hpp"    // validation::
#include "log/Logger.hpp"            // CURRENT_FILE_NAME
#include <efsw/efsw.hpp>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

static Logger& LOG() {
  static auto logger = Logger(CURRENT_FILE_NAME);
  return logger;
}

struct [[nodiscard]] DirWatcherImpl final : DirWatcher, efsw::FileWatchListener {
private:
  // Memory layout optimized: largest to smallest to minimize padding
  efsw::FileWatcher m_watcher;
  std::function<void(const fs::path&)> m_callback;
  fs::path m_dir;
  std::mutex m_callbackMutex;
  efsw::WatchID m_watchId;
  std::atomic<bool> m_stopped {true};

  // Implementation of efsw::FileWatchListener interface
  void handleFileAction([[maybe_unused]] efsw::WatchID watchid, const std::string& dir,
                        const std::string& filename, efsw::Action action,
                        [[maybe_unused]] std::string oldFilename) override {
    // Fast path: early filter on action and extension before constructing fs::path
    if ((efsw::Actions::Modified != action and efsw::Actions::Add != action) or
        !filename.ends_with(".txt")) {
      return;
    }

    // Construct path only when we know it's a valid event
    const fs::path filePath(dir + "/" + filename);

    const auto actionStr = efsw::Actions::Modified == action ? "Modified" : "Add";
    LOG().info<"The file {} has changed (action: {}), notify listener">(filePath.string(),
                                                                        actionStr);

    // Call user callback in thread-safe manner
    const std::scoped_lock lock(m_callbackMutex);
    
    if (!m_stopped.load() and m_callback) {
      m_callback(filePath);
    }
  }

public:
  explicit DirWatcherImpl(const fs::path& dir)
    : m_dir {dir},
      m_watchId {-1} {
    validation::require(pf::isDir(m_dir), "the dir provided to DirWatcher() is not valid.");
    LOG().info<"will watch directory {} using EFSW (event-driven)">(dir.string());
  }

  DirWatcherImpl(const DirWatcherImpl&) = delete;
  DirWatcherImpl(DirWatcherImpl&&) = delete;
  DirWatcherImpl& operator=(const DirWatcherImpl&) = delete;
  DirWatcherImpl& operator=(DirWatcherImpl&&) = delete;
  ~DirWatcherImpl() = default;

  void start(const std::function<void(const fs::path&)>& fileHasChangedCb) override {
    std::scoped_lock lock(m_callbackMutex);
    m_callback = fileHasChangedCb;
    const auto isRecursive = false;
    m_watchId = m_watcher.addWatch(m_dir.string(), this, isRecursive);

    if (0 > m_watchId) {
      LOG().error<"Failed to add watch for directory {}">(m_dir.string());
      return;
    }

    m_watcher.watch();
    m_stopped = false;
    LOG().info<"Started watching directory {} (WatchID: {})">(m_dir.string(), m_watchId);
  }

  void stop() override {
    if (!m_stopped.load()) {
      m_stopped = true;
      m_watcher.removeWatch(m_watchId);

      // Ensure all pending callbacks are finished before returning
      // Lock and unlock the mutex to synchronize with any running callback
      const std::scoped_lock lock(m_callbackMutex);
      m_callback = nullptr;
      LOG().info<"Stopped watching directory {}">(m_dir.string());
    }
  }

  [[nodiscard]] bool isStopped() const noexcept override { return m_stopped.load(); }
}; // class DirWatcherImpl

DirWatcher::~DirWatcher() = default;

[[nodiscard]] std::unique_ptr<DirWatcher> DirWatcher::create(const fs::path& dir) {
  return std::make_unique<DirWatcherImpl>(dir);
}
