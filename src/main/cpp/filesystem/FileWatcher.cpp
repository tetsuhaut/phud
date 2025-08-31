#include "filesystem/FileUtils.hpp"
#include "filesystem/FileWatcher.hpp" // std::chrono, toMilliseconds, std::filesystem::path, std::string, toString, count
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PeriodicTask.hpp"

#include <unordered_map>
#include <system_error> // std::error_code

namespace fs = std::filesystem;
static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] FileWatcher::Implementation final {
  fs::path m_file;
  std::error_code m_errorCode {};
  // note: impossible to use std::filesystem::path as a map key
  fs::file_time_type m_lastModifDate;
  PeriodicTask m_task;

  Implementation(std::chrono::milliseconds reloadPeriod, const fs::path& file) :
    m_file { file },
    m_lastModifDate { fs::last_write_time(file, m_errorCode) },
    m_task { reloadPeriod, CURRENT_FILE_NAME } {
    if (0 != m_errorCode.value()) {
      LOG.error<"Error getting last write time for file {} in directory {}: {}">(
        file.string(), file.parent_path().string(), m_errorCode.message());
    }
  }
  Implementation(int, auto) = delete; // use only path
};

// look at the file, take its last modification date, if it has changed notify
// the listener through the callback
static inline void getLatestUpdatedFile(const fs::path& file,
                                        fs::file_time_type& lastModified, auto&& fileHasChangedCb) {
  std::error_code ec;

  if (const auto & lasWriteTime { fs::last_write_time(file, ec) }; 0 == ec.value()) {
    if (lastModified != lasWriteTime) {
      lastModified = lasWriteTime;
      LOG.info<"The file\n{}\nhas changed, notify listener">(file.string());
      std::forward<decltype(fileHasChangedCb)>(fileHasChangedCb)(file);
    }
  } else [[unlikely]] {
    LOG.error<"Error checking if the file {} has changed: {}">(file.string(), ec.message());
  }
}

static inline void getLatestUpdatedFile(auto, fs::file_time_type&, auto&&) = delete;

FileWatcher::FileWatcher(std::chrono::milliseconds reloadPeriod, const fs::path& file)
  : m_pImpl{ std::make_unique<FileWatcher::Implementation>(reloadPeriod, file) } {
  phudAssert(phud::filesystem::isFile(m_pImpl->m_file),
             "the file provided to FileWatcher() is not valid.");
  LOG.info<"will watch the file {} every {}ms">(m_pImpl->m_file.string(), reloadPeriod.count());
}

FileWatcher::~FileWatcher() = default;

void FileWatcher::start(std::function<void(const fs::path&)> fileHasChangedCb) {
  phudAssert(nullptr != fileHasChangedCb, "null callback in FileWatcher::start()");
  m_pImpl->m_task.start([this, fileHasChangedCb]() {
    getLatestUpdatedFile(m_pImpl->m_file, m_pImpl->m_lastModifDate, fileHasChangedCb);
    return isStopped() ? PeriodicTaskStatus::stopTask : PeriodicTaskStatus::repeatTask;
  });
}

void FileWatcher::stop() { m_pImpl->m_task.stop(); }

/*[[nodiscard]]*/ bool FileWatcher::isStopped() const noexcept { return m_pImpl->m_task.isStopped(); }