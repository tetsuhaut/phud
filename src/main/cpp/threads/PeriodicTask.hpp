#pragma once

#include <chrono>
#include <functional> // std::function
#include <memory>     // std::unique_ptr
#include <string_view>

enum class /*[[nodiscard]]*/ PeriodicTaskStatus : short { repeatTask, stopTask };

[[nodiscard]] constexpr std::string_view toString(PeriodicTaskStatus status) noexcept {
  return PeriodicTaskStatus::repeatTask == status ? "repeatTask" : "stopTask";
}

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
  void stop() const;

  /**
   * Blocks until the given task returns false or stop() is called.
   */
  void join() const;
  [[nodiscard]] bool isStopped() const noexcept;
  [[nodiscard]] bool isRunning() const noexcept;
  /**
   * if task returns PeriodicTaskStatus::repeatTask, continue, else stop.
   */
  void start(const std::function<PeriodicTaskStatus()>& task) const;
}; // class PeriodicTask
