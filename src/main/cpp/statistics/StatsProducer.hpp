#pragma once

#include "threads/ThreadSafeQueue.hpp" // std::unique_ptr

#include <chrono>

class Database;
struct TableStatistics;

class [[nodiscard]] StatsProducer final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  struct [[nodiscard]] StatsProducerArgs final {
    std::chrono::milliseconds reloadPeriod; std::string_view site;
    std::string_view tableWindowName; const Database& db;
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
