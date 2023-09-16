#pragma once

#include "containers/ThreadSafeQueue.hpp" // uptr
#include <chrono>
#include <functional> // std::function

struct TableStatistics;

class [[nodiscard]] StatsConsumer final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  StatsConsumer(std::chrono::milliseconds reloadPeriod, ThreadSafeQueue<TableStatistics>& stats);
  StatsConsumer(const StatsConsumer&) = delete;
  StatsConsumer(StatsConsumer&&) = delete;
  StatsConsumer& operator=(const StatsConsumer&) = delete;
  StatsConsumer& operator=(StatsConsumer&&) = delete;
  ~StatsConsumer();
  /**
   * Calls @param observer each time a new TableStatistics is found it the stats queue.
   */
  void consumeAndNotify(std::function<void(TableStatistics&)> observer);
  void stop();
  [[nodiscard]] bool isStopped() const noexcept;
}; // class StatsConsumer
