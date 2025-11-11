#pragma once

#include <chrono>
#include <functional> // std::function
#include <memory> // std::unique_ptr

// forward declarations
struct TableStatistics;
template <typename T>
class ThreadSafeQueue;

class [[nodiscard]] StatsConsumer final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  StatsConsumer(std::chrono::milliseconds reloadPeriod, ThreadSafeQueue<TableStatistics>& stats);
  StatsConsumer(const StatsConsumer&) = delete;
  StatsConsumer(StatsConsumer&&) = delete;
  StatsConsumer& operator=(const StatsConsumer&) = delete;
  StatsConsumer& operator=(StatsConsumer&&) = delete;
  ~StatsConsumer();
  /**
   * @param observerCb the callback called
   * Calls observerCb each time a new TableStatistics is found it the stats queue.
   */
  void consumeAndNotify(const std::function<void(TableStatistics&)>& observerCb) const;
  void stop() const;
}; // class StatsConsumer
