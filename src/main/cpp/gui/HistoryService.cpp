#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "filesystem/DirWatcher.hpp"
#include "filesystem/FileUtils.hpp" // phud::filesystem::listTxtFilesInDir
#include "gui/HistoryService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "threads/ThreadPool.hpp"        // Future
#include <stlab/concurrency/utility.hpp> // stlab::await
#include <algorithm>                     // std::max
#include <atomic>
#include <deque>
#include <ranges>
#include <system_error> // std::error_code
#include <thread>       // std::thread::hardware_concurrency
#include <utility>      // std::pair

static constexpr Logger LOG {CURRENT_FILE_NAME};

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

namespace {
  /**
   * Called once for each file of an import, saved or not.
   */
  using FileDoneCallback = std::function<void(const fs::path& file, bool isSaved)>;

  /**
   * Saves the given site, which comes from one history file, in its own transaction.
   * @returns false if the site couldn't be read or saved
   */
  [[nodiscard]] bool saveSite(Database& database, const Site* pSite, const fs::path& file) {
    try {
      if (nullptr != pSite) {
        database.save(*pSite);
        return true;
      }
    } catch (const std::exception& e) {
      LOG.error<"Couldn't save the history file '{}': {}">(file.string(), e.what());
    }

    return false;
  }

  /**
   * Reads the given history file, and saves its hands into the database.
   * @returns false if the file couldn't be read or saved
   */
  [[nodiscard]] bool saveHistoryFile(PokerSiteHistory& history, Database& database,
                                     const fs::path& file) {
    const auto pSite = history.reloadFile(file);
    return saveSite(database, pSite.get(), file);
  }

  /**
   * @returns how many files are read in advance while the previous ones are saved: enough to keep
   * all the cores busy, without loading the whole history in memory.
   */
  [[nodiscard]] std::size_t getNbFilesReadInAdvance() noexcept {
    return std::max<std::size_t>(2, 2 * std::size_t {std::thread::hardware_concurrency()});
  }

  /**
   * Reads the given files in parallel, and saves each of them as soon as it is read, in its own
   * transaction: an interrupted import keeps the files already saved. The files are saved one by
   * one, as a SQLite connection executes one query at a time.
   * @param stop checked between two files: when it is true, the files not saved yet are ignored
   */
  void readAndSaveFiles(const std::shared_ptr<PokerSiteHistory>& pHistory, Database& database,
                        std::span<const fs::path> files, const std::atomic_bool& stop,
                        const FileDoneCallback& onFileDone) {
    std::deque<std::pair<fs::path, Future<Site*>>> pendingReads;
    auto nextFile = files.begin();
    const auto readNextFile = [&pendingReads, &nextFile, &pHistory]() {
      // Site* and not std::unique_ptr<Site>, as a stlab future result must be copyable
      pendingReads.emplace_back(*nextFile, ThreadPool::submit([pHistory, file = *nextFile]() {
        return pHistory->reloadFile(file).release();
      }));
      ++nextFile;
    };
    const auto nbFilesReadInAdvance = getNbFilesReadInAdvance();

    while (files.end() != nextFile and nbFilesReadInAdvance > pendingReads.size()) {
      readNextFile();
    }

    while (!pendingReads.empty()) {
      auto [file, read] = std::move(pendingReads.front());
      pendingReads.pop_front();
      // always awaited, so that the read site is freed even after a stop
      const auto pSite = std::unique_ptr<Site>(stlab::await(std::move(read)));

      if (!stop) {
        if (files.end() != nextFile) {
          readNextFile();
        }

        const auto isSaved = saveSite(database, pSite.get(), file);
        onFileDone(file, isSaved);
      }
    }
  }

  [[nodiscard]] bool wasModifiedAfter(const fs::path& file,
                                      std::chrono::system_clock::time_point time) {
    std::error_code ec;
    const auto lastWrite = fs::last_write_time(file, ec);
    return !ec and time < std::chrono::clock_cast<std::chrono::system_clock>(lastWrite);
  }
} // anonymous namespace

struct [[nodiscard]] HistoryService::Implementation final {
  Database& m_database;
  std::shared_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_loadTask {};
  Future<void> m_catchUpTask {};
  std::unique_ptr<DirWatcher> m_dirWatcher {};
  fs::path m_historyDir {};
  std::atomic_bool m_stopImports {false};

  explicit Implementation(Database& database) noexcept
    : m_database {database} {}
};

HistoryService::HistoryService(Database& database)
  : m_pImpl {std::make_unique<Implementation>(database)} {}

HistoryService::~HistoryService() {
  try {
    stopWatchingHistory();
    HistoryService::stopImportingHistory();
  } catch (...) {
    LOG.error<
        "Unknown Error when stopping the history import in the HistoryService destruction.">();
  }
}

bool HistoryService::isValidHistory(const fs::path& dir) {
  return PokerSiteHistory::isValidHistory(dir);
}

void HistoryService::importHistory(const fs::path& dir, const std::function<void()>& onProgress,
                                   const std::function<void(std::size_t)>& onSetNbFiles,
                                   const std::function<void()>& onDone) {
  m_pImpl->m_historyDir = dir.lexically_normal();
  m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(dir);
  m_pImpl->m_stopImports = false;
  m_pImpl->m_loadTask =
      ThreadPool::submit([this, pHistory = m_pImpl->m_pokerSiteHistory, dir, onProgress,
                          onSetNbFiles]() {
        // as this method will execute in another thread, it should not throw
        try {
          const auto files = pf::listTxtFilesInDir(dir / "history");
          LOG.info<"Importing the {} files of the history directory '{}'">(files.size(),
                                                                           dir.string());

          if (onSetNbFiles and !files.empty()) {
            onSetNbFiles(files.size());
          }

          if (nullptr != pHistory) {
            readAndSaveFiles(pHistory, m_pImpl->m_database, files, m_pImpl->m_stopImports,
                             [&onProgress](const fs::path&, bool) {
                               if (onProgress) {
                                 onProgress();
                               }
                             });
          }
        } catch (const std::exception& e) {
          LOG.error<"Unexpected exception during the history import: {}.">(e.what());
        } catch (...) {
          LOG.error<"Unknown Error during the history import.">();
        }
      }).then([this, onDone]() {
        if (!m_pImpl->m_stopImports and onDone) {
          onDone();
        }
      });
}

void HistoryService::importFilesModifiedSince(const fs::path& dir,
                                              std::chrono::system_clock::time_point since,
                                              const FileSavedCallback& onFileSaved,
                                              const std::function<void()>& onDone) {
  const std::shared_ptr<PokerSiteHistory> pHistory = PokerSiteHistory::newInstance(dir);

  if (nullptr == pHistory) {
    LOG.error<"Can't catch up with the history directory '{}'">(dir.string());
    return;
  }

  m_pImpl->m_stopImports = false;
  m_pImpl->m_catchUpTask = ThreadPool::submit([this, pHistory, dir, since, onFileSaved, onDone]() {
    const auto files =
        pf::listTxtFilesInDir(dir / "history") |
        std::views::filter([since](const auto& f) { return wasModifiedAfter(f, since); }) |
        std::ranges::to<std::vector>();
    LOG.info<"Importing {} history files modified since the last check">(files.size());
    readAndSaveFiles(pHistory, m_pImpl->m_database, files, m_pImpl->m_stopImports,
                     [&onFileSaved](const fs::path& file, bool isSaved) {
                       if (isSaved and onFileSaved) {
                         onFileSaved(file);
                       }
                     });

    if (!m_pImpl->m_stopImports and onDone) {
      onDone();
    }
  });
}

void HistoryService::watchHistory(const fs::path& dir, const FileSavedCallback& onFileSaved) {
  stopWatchingHistory();
  const std::shared_ptr<PokerSiteHistory> pHistory = PokerSiteHistory::newInstance(dir);

  if (nullptr == pHistory) {
    LOG.error<"Can't watch the history directory '{}'">(dir.string());
    return;
  }

  LOG.info<"Watching the history directory '{}'">(dir.string());
  m_pImpl->m_dirWatcher = DirWatcher::create(dir / "history");
  m_pImpl->m_dirWatcher->start(
      [pHistory, &database = m_pImpl->m_database, onFileSaved](const fs::path& file) {
        if (saveHistoryFile(*pHistory, database, file) and onFileSaved) {
          onFileSaved(file);
        }
      });
}

void HistoryService::stopWatchingHistory() {
  if (nullptr != m_pImpl->m_dirWatcher) {
    m_pImpl->m_dirWatcher->stop(); // waits for the end of the callback in progress
    m_pImpl->m_dirWatcher.reset();
  }
}

void HistoryService::setHistoryDir(const fs::path& dir) {
  m_pImpl->m_historyDir = dir;
  LOG.info<"Chosen Poker Site History Directory: {}">(dir.string());
  m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(m_pImpl->m_historyDir);
}

void HistoryService::stopImportingHistory() {
  // the imports stop after the file being saved: what is already saved is kept
  m_pImpl->m_stopImports = true;

  if (m_pImpl->m_loadTask.valid()) {
    stlab::await(std::move(m_pImpl->m_loadTask));
  }

  m_pImpl->m_loadTask.reset();

  if (m_pImpl->m_catchUpTask.valid()) {
    stlab::await(std::move(m_pImpl->m_catchUpTask));
  }

  m_pImpl->m_catchUpTask.reset();
}

std::shared_ptr<PokerSiteHistory> HistoryService::getPokerSiteHistory() const noexcept {
  return m_pImpl->m_pokerSiteHistory;
}
