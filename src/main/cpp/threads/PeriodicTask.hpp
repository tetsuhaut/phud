#pragma once

#include <chrono>
#include <functional> // std::function
#include <memory> // std::unique_ptr
#include <string_view>

enum class /*[[nodiscard]]*/ PeriodicTaskStatus : short {
  repeatTask,
  stopTask
};

std::string_view toString(PeriodicTaskStatus);

class [[nodiscard]] PeriodicTask final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

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
   * if task returns PeriodicTaskStatus::repeatTask, continue, else stop.
   */
  void start(std::function<PeriodicTaskStatus()> task);
}; // class PeriodicTask