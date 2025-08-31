#include "db/Database.hpp"
#include "entities/Site.hpp"
#include "filesystem/FileWatcher.hpp"
#include "gui/Gui.hpp"
#include "history/PokerSiteHistory.hpp"
#include "language/Either.hpp" // ErrOrRes
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/App.hpp" // App, std::unique_ptr, std::filesystem::path, phud::filesystem::*
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsConsumer.hpp" // ThreadSafeQueue
#include "statistics/StatsProducer.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp" // Future, std::chrono::*

#include <stlab/concurrency/utility.hpp> // stlab::await

#include <chrono>

namespace fs = std::filesystem;

static Logger LOG { CURRENT_FILE_NAME };

static constexpr std::chrono::milliseconds RELOAD_PERIOD { 2000 };

struct [[nodiscard]] App::Implementation final {
  std::unique_ptr<Database> m_model;
  std::unique_ptr<Gui> m_gui {};
  ThreadSafeQueue<TableStatistics> m_statsQueue {};
  std::unique_ptr<FileWatcher> m_fileWatcher {};
  std::unique_ptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_reloadTask {};
  Future<void> m_loadTask {};
  fs::path historyDir {};

  Implementation(std::string_view databaseName)
    : m_model { std::make_unique<Database>(databaseName) } {}

  [[nodiscard]] static inline TableStatistics extractTableStatistics(auto& self,
      std::string_view table) {
    if (auto stats { self.m_model->readTableStatistics({.site = "Winamax", .table = table}) };
        stats.isValid()) {
      LOG.debug<"Got stats from db.">();
      return stats;
    } else {
      LOG.debug<"Got no stats from db yet for table {} on site {}.">(table, "Winamax");
      return TableStatistics();
    }
  }

  /**
   *  on regarde les fichiers dans l'historique, on recharge ceux qui sont mis à jour
   * on notifie l'observer (le GUI) des nouvelles stats
   */
  static void watchHistoFile(App::Implementation& self, const fs::path& file,
                                    std::string table, auto observer) {
    self.m_fileWatcher = std::make_unique<FileWatcher>(::RELOAD_PERIOD, file);
    self.m_fileWatcher->start([&self, table, &observer](const fs::path & f) {
      self.m_reloadTask = ThreadPool::submit([&self, table, &observer, f]() {
        LOG.debug<"Notified, reloading the file\n{}">(f.string());
        return self.m_pokerSiteHistory->reloadFile(f);
      })
      .then([&self](const auto & pSite) { self.m_model->save(*pSite); })
      .then([&self, table]() { return extractTableStatistics(self, table); })
      .then([&self, &observer](TableStatistics&& ts) { notify(std::move(ts), observer); });
    });
    self.m_gui->informUser(fmt::format("Watching the file {}", file.filename().string()));
  }
};

App::App(std::string_view databaseName)
  : m_pImpl {std::make_unique<Implementation>(databaseName)} {}

App::~App() {
  try {
    if (nullptr != m_pImpl->m_fileWatcher) { m_pImpl->m_fileWatcher->stop(); }
  } catch (...) { // can't throw in a destructor
    LOG.error<"Unknown Error when stopping the file watch in the App destruction.">();
  }
  try {
    stopImportingHistory();
  } catch (...) { // can't throw in a destructor
    LOG.error<"Unknown Error when stopping the history import in the App destruction.">();
  }
  try {
    stopProducingStats();
  } catch (...) { // can't throw in a destructor
    LOG.error<"Unknown Error when stopping the stat production in the App destruction.">();
  }
}

static inline void notify(TableStatistics&& stats, auto observer) {
  if (Seat::seatUnknown == stats.getMaxSeat()) {
    LOG.debug<"Got no stats from db.">();
  } else {
    LOG.debug<"Got {} player stats objects.">(tableSeat::toInt(stats.getMaxSeat()));
    observer(std::move(stats));
  }
}

int App::showGui() { /*override*/
  m_pImpl->m_gui = std::make_unique<Gui>(*this);
  return m_pImpl->m_gui->run();
}

void App::importHistory(const fs::path& historyDir,
                        std::function<void()> incrementCb,
                        std::function<void(std::size_t)> setNbFilesCb,
                        std::function<void()> doneCb) {
  m_pImpl->historyDir = historyDir.lexically_normal();
  m_pImpl->m_loadTask = ThreadPool::submit([this, historyDir, incrementCb, setNbFilesCb]() {
    // as this method will execute in another thread, it should not throw
    try {
      if (m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(historyDir);
          m_pImpl->m_pokerSiteHistory) {
        return m_pImpl->m_pokerSiteHistory->load(historyDir, incrementCb, setNbFilesCb);
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
      if (pSite) { m_pImpl->m_model->save(*pSite); }
    } catch (const DatabaseException& e) {
      LOG.error<"Exception during the database usage: {}.">(e.what());
    } catch (...) { LOG.error<"Unknown during the database usage.">(); }
  })
  .then([doneCb]() { if (doneCb) { doneCb(); }});
}

// TODO utiliser des variables atomiques pour limiter ce que fait le load
void App::stopImportingHistory() {
  if (m_pImpl->m_pokerSiteHistory) { m_pImpl->m_pokerSiteHistory->stopLoading(); }

  if (m_pImpl->m_loadTask.valid()) { stlab::await(std::move(m_pImpl->m_loadTask)); }

  m_pImpl->m_loadTask.reset();
}

// on écoute les changements du fichier d'historique,
// en cas de changement on requête périodiquement la base pour recuperer les stats
std::string App::startProducingStats(std::string_view tableWindowTitle,
                                     std::function < void(TableStatistics&& ts) > observer) {
  const auto& h { m_pImpl->m_pokerSiteHistory->getHistoryFileFromTableWindowTitle(m_pImpl->historyDir, tableWindowTitle) };

  if (h.empty()) { return fmt::format("Couldn't get history file for table '{}'", tableWindowTitle); }

  const auto tableName { m_pImpl->m_pokerSiteHistory->getTableNameFromTableWindowTitle(tableWindowTitle) };
  App::Implementation::watchHistoFile(*m_pImpl, h, std::string(tableName), observer);
  return "";
}

void App::stopProducingStats() {
  if (nullptr != m_pImpl->m_fileWatcher) { m_pImpl->m_fileWatcher->stop(); }
}

void App::setHistoryDir(const fs::path& historyDir) {
  phudAssert(nullptr == m_pImpl->m_pokerSiteHistory, "m_pImpl->m_pokerSiteHistory is not empty");
  m_pImpl->historyDir = historyDir;
  m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(m_pImpl->historyDir);
}