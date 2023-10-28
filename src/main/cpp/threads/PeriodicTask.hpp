#pragma once

#include "strings/StringUtils.hpp"
#include "system/memory.hpp" // uptr
#include <chrono>
#include <functional> // std::function 

class [[nodiscard]] PeriodicTask final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  explicit PeriodicTask(std::chrono::milliseconds period, std::string_view taskName = "");
  PeriodicTask(const PeriodicTask&) = delete;
  PeriodicTask(PeriodicTask&&) = delete;
  PeriodicTask& operator=(const PeriodicTask&) = delete;
  PeriodicTask& operator=(PeriodicTask&&) = delete;
  ~PeriodicTask();

  /*
   * Stops the task as soon as possible
   */
  void stop();

  /**
   * Blocks until the given task returns false or stop() is called.
   */
  void join();
  [[nodiscard]] bool isStopped() const noexcept;
  /**
   * if task returns true, continue, else stop.
   */
  void start(std::function<bool()> task);
}; // class PeriodicTask