#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "statistics/StatsConsumer.hpp"
#include "statistics/PlayerStatistics.hpp" // required by TableStatistics
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadSafeQueue.hpp"

static Logger& LOG() {
  static auto logger = Logger(CURRENT_FILE_NAME);
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
    if (auto oStats = m_pImpl->m_statsQueue.tryPop(); oStats.has_value()) {
      if (Seat::seatUnknown == oStats->getMaxSeat()) {
        LOG().debug<"Got no stats from db.">();
      } else {
        LOG().debug<"Got {} player stats objects.">(tableSeat::toInt(oStats->getMaxSeat()));
        observerCb(*oStats);
      }
    }

    return PeriodicTaskStatus::repeatTask;
  });
}

void StatsConsumer::stop() const {
  m_pImpl->m_task.stop();
}
