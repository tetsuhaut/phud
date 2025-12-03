#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "statistics/StatsConsumer.hpp"
#include "statistics/PlayerStatistics.hpp" // required by TableStatistics
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadSafeQueue.hpp"

static Logger& LOG() {
  static Logger logger {CURRENT_FILE_NAME};
  return logger;
}

struct [[nodiscard]] StatsConsumer::Implementation final {
  ThreadSafeQueue<TableStatistics>& m_statsQueue;
  PeriodicTask m_task;

  Implementation(std::chrono::milliseconds reloadPeriod,
                 ThreadSafeQueue<TableStatistics>& statsQueue)
    : m_statsQueue {statsQueue},
      m_task {reloadPeriod, CURRENT_FILE_NAME} {}
};

StatsConsumer::StatsConsumer(std::chrono::milliseconds reloadPeriod,
                             ThreadSafeQueue<TableStatistics>& stats)
  : m_pImpl {std::make_unique<Implementation>(reloadPeriod, stats)} {}

StatsConsumer::~StatsConsumer() = default;

void StatsConsumer::consumeAndNotify(
    const std::function<void(TableStatistics&)>& observerCb) const {
  m_pImpl->m_task.start([this, observerCb]() {
    if (TableStatistics stats; m_pImpl->m_statsQueue.tryPop(stats)) {
      if (Seat::seatUnknown == stats.getMaxSeat()) {
        LOG().debug<"Got no stats from db.">();
      } else {
        LOG().debug<"Got {} player stats objects.">(tableSeat::toInt(stats.getMaxSeat()));
        observerCb(stats);
      }
    }

    return PeriodicTaskStatus::repeatTask;
  });
}

void StatsConsumer::stop() const {
  m_pImpl->m_task.stop();
}
