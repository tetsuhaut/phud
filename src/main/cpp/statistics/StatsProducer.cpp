#include "db/Database.hpp"
#include "language/Validator.hpp"
#include "log/Logger.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsProducer.hpp" // std::array, toMilliseconds, std::unique_ptr
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadSafeQueue.hpp"

static Logger& LOG() {
  static Logger logger {CURRENT_FILE_NAME};
  return logger;
}

struct [[nodiscard]] StatsProducer::Implementation final {
  PeriodicTask m_task;
  std::string m_site;
  std::string m_table;
  const Database& m_db;

  explicit Implementation(const StatsProducerArgs& args)
    : m_task {args.reloadPeriod, "StatsProducer"},
      m_site {args.site},
      m_table {args.tableWindowName},
      m_db {args.db} {
    validation::requireNonEmpty(args.site, "site");
    validation::requireNonEmpty(args.tableWindowName, "tableWindowName");
  }
};

StatsProducer::StatsProducer(const StatsProducerArgs& args)
  : m_pImpl {std::make_unique<Implementation>(args)} {}

StatsProducer::~StatsProducer() = default;

void StatsProducer::start(ThreadSafeQueue<TableStatistics>& statsQueue) const {
  m_pImpl->m_task.start([this, &statsQueue]() {
    if (auto stats {m_pImpl->m_db.readTableStatistics(m_pImpl->m_site, m_pImpl->m_table)};
        stats.isValid()) {
      LOG().debug<"Got stats from db.">();
      statsQueue.push(std::move(stats));
    } else {
      LOG().debug<"Got no stats from db yet for table {} on site {}.">(m_pImpl->m_table,
                                                                       m_pImpl->m_site);
    }
    return PeriodicTaskStatus::repeatTask;
  });
}

void StatsProducer::stop() const {
  m_pImpl->m_task.stop();
}

/*[[nodiscard]]*/
bool StatsProducer::isStopped() const noexcept {
  return m_pImpl->m_task.isStopped();
}
