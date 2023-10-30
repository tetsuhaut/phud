#include "containers/algorithms.hpp" // forEach
#include <unordered_map>
#include "filesystem/DirWatcher.hpp" // std::chrono, toMilliseconds, FileTime, Path, String, toString
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PeriodicTask.hpp" // NonCopyable

#include <concepts>
#include <system_error>

namespace fs = std::filesystem;
namespace pa = phud::algorithms;
namespace pf = phud::filesystem;

using FileTimes = std::unordered_map<std::string, std::filesystem::file_time_type>;

static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] DirWatcher::Implementation final {
  fs::path m_dir;
  // note: as of C++ 17, there is no portable way to unify std::filesystem::file_time_type and std::time :(
  // note: impossible to use Path as a map key
  FileTimes m_refFileToLastModifDate {};
  PeriodicTask m_task;

  Implementation(std::same_as<std::chrono::milliseconds> auto reloadPeriod, const fs::path& dir) :
    m_dir { dir },
  m_task { reloadPeriod, "DirWatcher" } {}
};

// look at each file, take its last modification date, wait, do it again and compare
// for each modified file, notify the listener through the callback
template<typename T> requires(std::same_as<T, fs::path>)
[[nodiscard]] static inline bool getLatestUpdatedFiles(const T& dir,
    FileTimes& ref, auto fileHasChangedCb) {
  LOG.trace<"Searching for file changes in dir {}">(dir.string());
  const auto& files { pf::listTxtFilesInDir(dir) };
  pa::forEach(files, [&ref, &fileHasChangedCb](const auto & file) {
    std::error_code ec;
    const auto& fileName{ file.string() };

    if (const auto & lasWriteTime { std::filesystem::last_write_time(file, ec) }; ec) {
      if ((0 == ref.count(fileName)) or (ref[fileName] != lasWriteTime)) {
        ref[fileName] = lasWriteTime;
        LOG.info<"The file {} has changed, notify listener">(fileName);
        fileHasChangedCb(file);
      }
    } else [[unlikely]] {
        LOG.error<"Error getting last write time for file {} in directory {}: {}">(
          fileName, file.parent_path().string(), ec.message());
      }
    });
  return true;
}

DirWatcher::DirWatcher(std::chrono::milliseconds reloadPeriod, const fs::path& dir)
  : m_pImpl{ std::make_unique<Implementation>(reloadPeriod, dir)} {
  phudAssert(pf::isDir(m_pImpl->m_dir),
             "the dir provided to DirWatcher() is not valid.");
  LOG.info<"will watch directory {} every {}ms">(dir.string(), reloadPeriod.count());
}

DirWatcher::~DirWatcher() = default;

void  DirWatcher::start(std::function<void(const fs::path&)> fileHasChangedCb) {
  m_pImpl->m_task.start([this, fileHasChangedCb]() {
    return getLatestUpdatedFiles(m_pImpl->m_dir, m_pImpl->m_refFileToLastModifDate, fileHasChangedCb);
  });
}

void DirWatcher::stop() { m_pImpl->m_task.stop(); }

/*[[nodiscard]]*/ bool DirWatcher::isStopped() const noexcept { return m_pImpl->m_task.isStopped(); }