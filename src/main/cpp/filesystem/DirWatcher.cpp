#include "filesystem/DirWatcher.hpp" // std::chrono, toMilliseconds, FileTime, std::filesystem::path, std::string, toString

#include "filesystem/FileUtils.hpp" // phud::filesystem::*
#include "language/Validator.hpp" // validation::
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PeriodicTask.hpp" // NonCopyable
#include <unordered_map>

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

using FileTimes = std::unordered_map<std::string, fs::file_time_type>;

static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] DirWatcher::Implementation final {
  fs::path m_dir;
  // note: as of C++ 17, there is no portable way to unify std::filesystem::file_time_type and std::time :(
  // note: impossible to use std::filesystem::path as a map key
  FileTimes m_refFileToLastModifDate {};
  PeriodicTask m_task;

  Implementation(std::same_as<std::chrono::milliseconds> auto reloadPeriod, fs::path dir) :
    m_dir { std::move(dir) },
    m_task { reloadPeriod, "DirWatcher" } {}
};

// look at each file, take its last modification date, wait, do it again and compare
// for each modified file, notify the listener through the callback
template <typename T> requires(std::same_as<T, fs::path>)
[[nodiscard]] static PeriodicTaskStatus getLatestUpdatedFiles(const T& dir,
                                                              FileTimes& ref, auto fileHasChangedCb) {
  LOG.trace<"Searching for file changes in dir {}">(dir.string());
  const auto& files { pf::listTxtFilesInDir(dir) };
  std::ranges::for_each(files, [&ref, &fileHasChangedCb](const auto& file) {
    std::error_code ec;
    const auto& fileName { file.string() };

    if (const auto& lasWriteTime { fs::last_write_time(file, ec) }; 0 == ec.value()) {
      if ((0 == ref.count(fileName)) or (ref[fileName] != lasWriteTime)) {
        ref[fileName] = lasWriteTime;
        LOG.info<"The file {} has changed, notify listener">(fileName);
        fileHasChangedCb(file);
      }
    }
    else [[unlikely]] {
      LOG.error<"Error getting last write time for file {} in directory {}: {}">(
        fileName, file.parent_path().string(), ec.message());
    }
  });
  return PeriodicTaskStatus::repeatTask;
}

DirWatcher::DirWatcher(std::chrono::milliseconds reloadPeriod, const fs::path& dir)
  : m_pImpl { std::make_unique<Implementation>(reloadPeriod, dir) } {
  validation::require(pf::isDir(m_pImpl->m_dir),
                      "the dir provided to DirWatcher() is not valid.");
  LOG.info<"will watch directory {} every {}ms">(dir.string(), reloadPeriod.count());
}

DirWatcher::~DirWatcher() = default;

void DirWatcher::start(const std::function<void(const fs::path&)>& fileHasChangedCb) const {
  m_pImpl->m_task.start([this, fileHasChangedCb]() {
    return getLatestUpdatedFiles(m_pImpl->m_dir, m_pImpl->m_refFileToLastModifDate, fileHasChangedCb);
  });
}

void DirWatcher::stop() const { m_pImpl->m_task.stop(); }

/*[[nodiscard]]*/
bool DirWatcher::isStopped() const noexcept { return m_pImpl->m_task.isStopped(); }
