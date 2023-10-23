#include "db/Database.hpp"
#include "entities/Game.hpp" // needed as Site declares incomplete CashGame type
#include "entities/Player.hpp" // needed as Site declares incomplete Player type
#include "entities/Site.hpp"
#include "filesystem/FileWatcher.hpp"
#include "gui/Gui.hpp"
#include "history/PokerSiteHistory.hpp"
#include "language/Either.hpp" // ErrOrRes
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/App.hpp" // App, uptr, Path, phud::filesystem::*
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsConsumer.hpp" // ThreadSafeQueue
#include "statistics/StatsProducer.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp" // Future, std::chrono::*

#include <stlab/concurrency/utility.hpp> // stlab::blocking_get

#include <chrono>

static Logger LOG { CURRENT_FILE_NAME };

namespace pf = phud::filesystem;

static constexpr std::chrono::milliseconds RELOAD_PERIOD { 2000 };

struct [[nodiscard]] App::Implementation final {
  uptr<Database> m_model;
  uptr<Gui> m_gui {};
  ThreadSafeQueue<TableStatistics> m_statsQueue {};
  uptr<FileWatcher> m_fileWatcher {};
  uptr<PokerSiteHistory> m_pokerSiteHistory {};
  Future<void> m_reloadTask {};
  Future<void> m_loadTask {};
  Path historyDir {};

  Implementation(StringView databaseName)
    : m_model { mkUptr<Database>(databaseName) } {}
};

App::App(StringView databaseName)
  : m_pImpl {mkUptr<Implementation>(databaseName)} {}

App::~App() {
  try {
    if (nullptr != m_pImpl->m_fileWatcher) { m_pImpl->m_fileWatcher->stop(); }

    stopImportingHistory();
    stopProducingStats();
  } catch (...) {
    std::exit(5);
  }
}

static inline [[nodiscard]] TableStatistics extractTableStatistics(auto& self, StringView table) {
  if (auto stats { self.m_model->readTableStatistics( {.site = "Winamax", .table = table}) };
    stats.isValid()) {
    LOG.debug<"Got stats from db.">();
    return stats;
  } else {
    LOG.debug<"Got no stats from db yet for table {} on site {}.">(table, "Winamax");
    return TableStatistics();
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

// on regarde les fichiers dans l'historique, on recharge ceux qui sont mis à jour
// on notifie l'observer (le GUI) des nouvelles stats
static inline void watchHistoFile(App::Implementation& self, const Path& file, String table, auto observer) {
  self.m_fileWatcher = mkUptr<FileWatcher>(::RELOAD_PERIOD, file);
  self.m_fileWatcher->start([&self, table, &observer](const Path & f) {
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

int App::showGui() { /*override*/
  m_pImpl->m_gui = mkUptr<Gui>(*this);
  return m_pImpl->m_gui->run();
}

void App::importHistory(const Path& historyDir,
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

    return uptr<Site>();
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

  if (m_pImpl->m_loadTask.valid()) { stlab::await(m_pImpl->m_loadTask); }

  m_pImpl->m_loadTask.reset();
}

// on écoute les changements du fichier d'historique,
// en cas de changement on requête périodiquement la base pour recuperer les stats
String App::startProducingStats(StringView tableWindowTitle,
  std::function<void(TableStatistics&& ts)> observer) {
  const auto& h { m_pImpl->m_pokerSiteHistory->getHistoryFileFromTableWindowTitle(m_pImpl->historyDir, tableWindowTitle) };

  if (h.empty()) { return fmt::format("Couldn't get history file for table '{}'", tableWindowTitle); }

  const auto tableName { m_pImpl->m_pokerSiteHistory->getTableNameFromTableWindowTitle(tableWindowTitle) };
  watchHistoFile(*m_pImpl, h, String(tableName), observer);
  return "";
}

void App::stopProducingStats() {
  if (nullptr != m_pImpl->m_fileWatcher) { m_pImpl->m_fileWatcher->stop(); }
}

void App::setHistoryDir(const Path& historyDir) {
  phudAssert(nullptr == m_pImpl->m_pokerSiteHistory, "m_pImpl->m_pokerSiteHistory is not empty");
  m_pImpl->historyDir = historyDir;
  m_pImpl->m_pokerSiteHistory = PokerSiteHistory::newInstance(m_pImpl->historyDir);
}