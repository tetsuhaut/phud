#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "statistics/StatsConsumer.hpp"
#include "statistics/PlayerStatistics.hpp" // required by TableStatistics
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"

static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] StatsConsumer::Implementation final {
  ThreadSafeQueue<TableStatistics>& m_statsQueue;
  PeriodicTask m_task;

  Implementation(std::chrono::milliseconds reloadPeriod, ThreadSafeQueue<TableStatistics>& statsQueue)
    : m_statsQueue { statsQueue },
      m_task { reloadPeriod, CURRENT_FILE_NAME } {}
};

StatsConsumer::StatsConsumer(std::chrono::milliseconds reloadPeriod,
                             ThreadSafeQueue<TableStatistics>& statsQueue)
  : m_pImpl { mkUptr<Implementation>(reloadPeriod, statsQueue) } {}

StatsConsumer::~StatsConsumer() = default;

void StatsConsumer::consumeAndNotify(std::function<void(TableStatistics&)> observerCb) {
  m_pImpl->m_task.start([this, observerCb]() {
    if (TableStatistics stats; m_pImpl->m_statsQueue.tryPop(stats)) {
      if (Seat::seatUnknown == stats.getMaxSeat()) {
        LOG.debug<"Got no stats from db.">();
      } else {
        LOG.debug(fmt::format("Got {} player stats objects.", stats.getMaxSeat()));
        observerCb(stats);
      }
    }

    return true;
  });
}

void StatsConsumer::stop() { m_pImpl->m_task.stop(); }