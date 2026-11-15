#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadPool.hpp" // Future, std::atomic_bool

#include <condition_variable>
#include <mutex>

static constexpr Logger LOG {CURRENT_FILE_NAME};

struct [[nodiscard]] PeriodicTask::Implementation final {
  Future<void> m_futureTaskResult {};
  std::condition_variable m_cv {};
  std::mutex m_mutex {};
  std::string m_name;
  std::atomic_bool m_stop = false;
  std::atomic_bool m_taskIsStopped = true;
  std::chrono::milliseconds m_period;

  explicit Implementation(std::chrono::milliseconds period, std::string_view taskName)
    : m_name {taskName},
      m_period {period} {}
}; // struct PeriodicTask::Implementation

PeriodicTask::PeriodicTask(std::chrono::milliseconds period, std::string_view taskName)
  : m_pImpl {std::make_unique<Implementation>(period, taskName)} {}

PeriodicTask::~PeriodicTask() {
  try {
    stop();
  } catch (...) {
    LOG.error<"Unknown error during the stop of PeriodicTask.">();
  }
}

void PeriodicTask::stop() const {
  if (!m_pImpl->m_taskIsStopped) {
    {
      auto lock = std::unique_lock<std::mutex>(m_pImpl->m_mutex); // noexcept
      m_pImpl->m_stop = true;
      m_pImpl->m_cv.notify_all(); // noexcept
    }
    join(); // can throw :(
    m_pImpl->m_futureTaskResult.reset(); // noexcept
  }
}

void PeriodicTask::join() const {
  if (!m_pImpl->m_taskIsStopped) {
    auto lock = std::unique_lock<std::mutex>(m_pImpl->m_mutex);
    // wait is not noexcept
    m_pImpl->m_cv.wait(lock, [this]() noexcept { return m_pImpl->m_taskIsStopped.load(); });
  }
}

/*[[nodiscard]]*/
bool PeriodicTask::isStopped() const noexcept {
  return m_pImpl->m_taskIsStopped;
}
/*[[nodiscard]]*/
bool PeriodicTask::isRunning() const noexcept {
  return false == m_pImpl->m_taskIsStopped;
}

void PeriodicTask::start(const std::function<PeriodicTaskStatus()>& task) const {
  // a previous stop() must not prevent this new start
  m_pImpl->m_stop = false;
  m_pImpl->m_taskIsStopped = false;
  m_pImpl->m_futureTaskResult = ThreadPool::submit([this, task]() {
    // returns true if the period has elapsed, false if stop() has been called
    const auto periodHasElapsed = [this]() {
      auto lock = std::unique_lock<std::mutex>(m_pImpl->m_mutex);
      const auto timeout = std::chrono::steady_clock::now() + m_pImpl->m_period;
      // it is possible that m_pImpl->m_cv wakes up without being notified (spuriously)
      // so when m_pImpl->m_cv wakes up, we check that stop is true or the timeout is reached.
      // wait_until() will return the value of m_stop
      return !m_pImpl->m_cv.wait_until(lock, timeout,
                                       [this]() noexcept { return m_pImpl->m_stop.load(); });
    };

    while (periodHasElapsed() and PeriodicTaskStatus::repeatTask == task()) {
      // the task is run by the loop condition
    }

    m_pImpl->m_taskIsStopped = true;
    m_pImpl->m_cv.notify_one();
  });
}
