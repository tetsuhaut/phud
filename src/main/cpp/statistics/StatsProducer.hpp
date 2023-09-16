#pragma once

#include "containers/ThreadSafeQueue.hpp" // uptr
#include <chrono>

class Database;
struct TableStatistics;

class [[nodiscard]] StatsProducer final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  struct [[nodiscard]] StatsProducerArgs final {
    std::chrono::milliseconds reloadPeriod; StringView site;
    StringView tableWindowName; const Database& db;
  };
  StatsProducer(const StatsProducerArgs& args);
  StatsProducer(const StatsProducer&) = delete;
  StatsProducer(StatsProducer&&) = delete;
  StatsProducer& operator=(const StatsProducer&) = delete;
  StatsProducer& operator=(StatsProducer&&) = delete;
  ~StatsProducer();
  /**
   * Watches table data. Each time a new one is created put it in @statsQueue.
   */
  void start(ThreadSafeQueue<TableStatistics>& statsQueue);
  void stop();
  [[nodiscard]] bool isStopped() const noexcept;
}; // class StatsProducer
