#include "filesystem/DirWatcher.hpp" // std::chrono, toMilliseconds, FileTime, std::filesystem::path, std::string, toString

#include "filesystem/FileUtils.hpp" // phud::filesystem::*
#include "language/Validator.hpp" // validation::
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PeriodicTask.hpp" // NonCopyable
#include <unordered_map>

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

using FileTimes = std::unordered_map<std::string, fs::file_time_type>;

static Logger& LOG() {
  static Logger logger { CURRENT_FILE_NAME };
  return logger;
}

DirWatcher::~DirWatcher() = default;

struct [[nodiscard]] DirWatcherImpl final : DirWatcher {
  fs::path m_dir;
  // note: as of C++ 17, there is no portable way to unify std::filesystem::file_time_type and std::time :(
  // note: impossible to use std::filesystem::path as a map key
  FileTimes m_refFileToLastModifDate {};
  PeriodicTask m_task;

  DirWatcherImpl(const DirWatcherImpl&) = delete;
  DirWatcherImpl(DirWatcherImpl&&) = delete;
  DirWatcherImpl& operator=(const DirWatcherImpl&) = delete;
  DirWatcherImpl& operator=(DirWatcherImpl&&) = delete;

  explicit DirWatcherImpl(std::chrono::milliseconds reloadPeriod, const fs::path& dir) :
    m_dir { std::move(dir) },
    m_task { reloadPeriod, "DirWatcher" } {
    validation::require(pf::isDir(m_dir),"the dir provided to DirWatcher() is not valid.");
    LOG().info<"will watch directory {} every {}ms">(dir.string(), reloadPeriod.count());
  }

  void callCb(const fs::path& file, const std::filesystem::file_time_type& lasWriteTime,  const std::function<void(const fs::path&)>& fileHasChangedCb) {
    if (!m_refFileToLastModifDate.contains(file.string()) or (m_refFileToLastModifDate[file.string()] != lasWriteTime)) {
      m_refFileToLastModifDate[file.string()] = lasWriteTime;
      LOG().info<"The file {} has changed, notify listener">(file.string());
      fileHasChangedCb(file);
    }
  }

  /** look at each file, take its last modification date, wait, do it again and compare
   * for each modified file, notify the listener through the callback
   */
  void start(const std::function<void(const fs::path&)>& fileHasChangedCb) override {
    m_task.start([this, fileHasChangedCb]() {
      LOG().trace<"Searching for file changes in dir {}">(m_dir.string());
      const auto& files { pf::listTxtFilesInDir(m_dir) };
      std::ranges::for_each(files, [this, &fileHasChangedCb](const auto& file) {
        std::error_code ec;

        if (const auto& lasWriteTime { fs::last_write_time(file, ec) }; 0 == ec.value()) {
          callCb(file, lasWriteTime, fileHasChangedCb);
        }
        else [[unlikely]] {
          LOG().error<"Error getting last write time for file {} in directory {}: {}">(
            file.string(), file.parent_path().string(), ec.message());
        }
      });
      return PeriodicTaskStatus::repeatTask;
    });
}

  void stop() const override { m_task.stop(); }

  [[nodiscard]] bool isStopped() const noexcept override { return m_task.isStopped(); }
};

[[nodiscard]] std::unique_ptr<DirWatcher> DirWatcher::create(std::chrono::milliseconds reloadPeriod, const fs::path& dir) {
  return std::make_unique<DirWatcherImpl>(reloadPeriod, dir);
}
