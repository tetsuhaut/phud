#include "containers/algorithms.hpp"
#include "db/Database.hpp"
#include "language/assert.hpp" // phudAssert
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsProducer.hpp" // ThreadSafeQueue, std::array, toMilliseconds, uptr
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"

static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] StatsProducer::Implementation final {
  PeriodicTask m_task;
  std::string m_site;
  std::string m_table;
  const Database& m_db;

  Implementation(const StatsProducerArgs& args)
    : m_task { args.reloadPeriod, "StatsProducer" },
      m_site { args.site },
      m_table { args.tableWindowName },
      m_db { args.db } {
    phudAssert(!args.site.empty(), "site is empty");
    phudAssert(!args.tableWindowName.empty(), "table is empty");
  }
};

StatsProducer::StatsProducer(const StatsProducerArgs& args)
  : m_pImpl { std::make_unique<Implementation>(args) } {}

StatsProducer::~StatsProducer() = default;

void StatsProducer::start(ThreadSafeQueue<TableStatistics>& statsQueue) {
  m_pImpl->m_task
  .start([this, &statsQueue]() {
    if (auto stats { m_pImpl->m_db.readTableStatistics(
    {.site = m_pImpl->m_site, .table = m_pImpl->m_table}) }; stats.isValid()) {
      LOG.debug<"Got stats from db.">();
      statsQueue.push(std::move(stats));
    } else { LOG.debug<"Got no stats from db yet for table {} on site {}.">(m_pImpl->m_table, m_pImpl->m_site); }

    return true;
  });
}

void StatsProducer::stop() { m_pImpl->m_task.stop(); }

/*[[nodiscard]]*/ bool StatsProducer::isStopped() const noexcept {
  return m_pImpl->m_task.isStopped();
}