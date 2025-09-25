#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "gui/HistoryService.hpp"
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp"
#include "threads/ThreadPool.hpp" // Future
#include <stlab/concurrency/utility.hpp> // stlab::await

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;

struct [[nodiscard]] HistoryService::Implementation final {
  Database& m_database;
  std::shared_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_loadTask {};
  fs::path m_historyDir {};

  explicit Implementation(Database& database)
    : m_database { database } {}
};

HistoryService::HistoryService(Database& database)
  : m_pImpl { std::make_unique<Implementation>(database) } {}

HistoryService::~HistoryService() {
  try {
    HistoryService::stopImportingHistory();
  } catch (...) {
    LOG.error<"Unknown Error when stopping the history import in the HistoryService destruction.">();
  }
}

bool HistoryService::isValidHistory(const fs::path& dir) {
  return PokerSiteHistory::isValidHistory(dir);
}

void HistoryService::importHistory(const fs::path& historyDir,
                                  std::function<void()> onProgress,
                                  std::function<void(std::size_t)> onSetNbFiles,
                                  std::function<void()> onDone) {
  m_pImpl->m_historyDir = historyDir.lexically_normal();
  m_pImpl->m_loadTask = ThreadPool::submit([this, historyDir, onProgress, onSetNbFiles]() {
    // as this method will execute in another thread, it should not throw
    try {
      if (m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(historyDir);
          m_pImpl->m_pokerSiteHistory) {
        return m_pImpl->m_pokerSiteHistory->load(historyDir, onProgress, onSetNbFiles);
      }
    } catch (const std::exception& e) {
      LOG.error<"Unexpected exception during the history import: {}.">(e.what());
    } catch (...) {
      LOG.error<"Unknown Error during the history import.">();
    }

    return std::unique_ptr<Site>();
  })
  .then([this](const auto & pSite) {
    try {
      if (pSite) { m_pImpl->m_database.save(*pSite); }
    } catch (const DatabaseException& e) {
      LOG.error<"Exception during the database usage: {}.">(e.what());
    } catch (...) { LOG.error<"Unknown during the database usage.">(); }
  })
  .then([onDone]() { if (onDone) { onDone(); }});
}

void HistoryService::setHistoryDir(const fs::path& historyDir) {
  m_pImpl->m_historyDir = historyDir;
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
}

std::shared_ptr<PokerSiteHistory> HistoryService::getPokerSiteHistory() const {
  return m_pImpl->m_pokerSiteHistory;
}
