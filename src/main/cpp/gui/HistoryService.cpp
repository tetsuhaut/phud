#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "filesystem/DirWatcher.hpp"
#include "filesystem/FileUtils.hpp" // phud::filesystem::listTxtFilesInDir
#include "gui/HistoryService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "threads/ThreadPool.hpp"        // Future
#include <stlab/concurrency/utility.hpp> // stlab::await
#include <atomic>
#include <ranges>
#include <system_error> // std::error_code

static Logger& LOG() {
  static auto logger = Logger(CURRENT_FILE_NAME);
  return logger;
}

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

namespace {
  /**
   * Reads the given history file, and saves its hands into the database.
   * @returns false if the file couldn't be read or saved
   */
  [[nodiscard]] bool saveHistoryFile(PokerSiteHistory& history, Database& database,
                                     const fs::path& file) {
    try {
      if (const auto pSite = history.reloadFile(file); nullptr != pSite) {
        database.save(*pSite);
        return true;
      }
    } catch (const std::exception& e) {
      LOG().error<"Couldn't save the history file '{}': {}">(file.string(), e.what());
    }

    return false;
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
  std::atomic_bool m_stopCatchUp {false};

  explicit Implementation(Database& database)
    : m_database {database} {}
};

HistoryService::HistoryService(Database& database)
  : m_pImpl {std::make_unique<Implementation>(database)} {}

HistoryService::~HistoryService() {
  try {
    stopWatchingHistory();
    HistoryService::stopImportingHistory();
  } catch (...) {
    LOG()
        .error<
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
  m_pImpl->m_loadTask =
      ThreadPool::submit([this, dir, onProgress, onSetNbFiles]() {
        // as this method will execute in another thread, it should not throw
        try {
          if (m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(dir);
              m_pImpl->m_pokerSiteHistory) {
            return m_pImpl->m_pokerSiteHistory->load(dir, onProgress, onSetNbFiles);
          }
        } catch (const std::exception& e) {
          LOG().error<"Unexpected exception during the history import: {}.">(e.what());
        } catch (...) {
          LOG().error<"Unknown Error during the history import.">();
        }

        return std::unique_ptr<Site>();
      })
          .then([this](const auto& pSite) {
            try {
              if (pSite) {
                m_pImpl->m_database.save(*pSite);
              }
            } catch (const DatabaseException& e) {
              LOG().error<"Exception during the database usage: {}.">(e.what());
            } catch (...) {
              LOG().error<"Unknown during the database usage.">();
            }
          })
          .then([onDone]() {
            if (onDone) {
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
    LOG().error<"Can't catch up with the history directory '{}'">(dir.string());
    return;
  }

  m_pImpl->m_stopCatchUp = false;
  m_pImpl->m_catchUpTask = ThreadPool::submit([this, pHistory, dir, since, onFileSaved, onDone]() {
    const auto files = pf::listTxtFilesInDir(dir / "history") |
                       std::views::filter([since](const auto& f) {
                         return wasModifiedAfter(f, since);
                       }) |
                       std::ranges::to<std::vector>();
    LOG().info<"Importing {} history files modified since the last check">(files.size());
    const auto whileNotStopped = [this](const auto&) { return !m_pImpl->m_stopCatchUp; };

    for (const auto& file : files | std::views::take_while(whileNotStopped)) {
      if (saveHistoryFile(*pHistory, m_pImpl->m_database, file) and onFileSaved) {
        onFileSaved(file);
      }
    }

    if (!m_pImpl->m_stopCatchUp and onDone) {
      onDone();
    }
  });
}

void HistoryService::watchHistory(const fs::path& dir, const FileSavedCallback& onFileSaved) {
  stopWatchingHistory();
  const std::shared_ptr<PokerSiteHistory> pHistory = PokerSiteHistory::newInstance(dir);

  if (nullptr == pHistory) {
    LOG().error<"Can't watch the history directory '{}'">(dir.string());
    return;
  }

  LOG().info<"Watching the history directory '{}'">(dir.string());
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
  LOG().info<"Chosen Poker Site History Directory: {}">(dir.string());
  m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(m_pImpl->m_historyDir);
}

void HistoryService::stopImportingHistory() {
  if (m_pImpl->m_pokerSiteHistory) {
    m_pImpl->m_pokerSiteHistory->stopLoading();
  }

  if (m_pImpl->m_loadTask.valid()) {
    stlab::await(std::move(m_pImpl->m_loadTask));
  }

  m_pImpl->m_loadTask.reset();
  m_pImpl->m_stopCatchUp = true;

  if (m_pImpl->m_catchUpTask.valid()) {
    stlab::await(std::move(m_pImpl->m_catchUpTask));
  }

  m_pImpl->m_catchUpTask.reset();
}

std::shared_ptr<PokerSiteHistory> HistoryService::getPokerSiteHistory() const {
  return m_pImpl->m_pokerSiteHistory;
}
