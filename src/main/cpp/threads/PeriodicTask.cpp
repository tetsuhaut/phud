#include "strings/String.hpp" // String
#include "threads/ConditionVariable.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadPool.hpp" // Future, std::atomic_bool
#include <stlab/concurrency/system_timer.hpp>

struct [[nodiscard]] PeriodicTask::Implementation final {
  Future<void> m_futureTaskResult {};
  ConditionVariable m_cv {};
  Mutex m_mutex {};
  String m_name;
  std::atomic_bool m_stop { false };
  std::atomic_bool m_taskIsStopped { true };
  std::chrono::milliseconds m_period;

  explicit Implementation(std::chrono::milliseconds period, StringView taskName)
    : m_name { taskName },
      m_period { period } {}
};

PeriodicTask::PeriodicTask(std::chrono::milliseconds period, StringView taskName)
  : m_pImpl { mkUptr<Implementation>(period, taskName) } {}

PeriodicTask::~PeriodicTask() { try { stop(); } catch (...) { std::exit(6); } }

void PeriodicTask::stop() {
  if (!m_pImpl->m_taskIsStopped) {
    {
      UniqueLock lock { m_pImpl->m_mutex }; // noexcept
      m_pImpl->m_futureTaskResult.reset(); // should be noexcept :)
      m_pImpl->m_stop = true;
      m_pImpl->m_cv.notify_all(); // noexcept
    }
    join();
  }
}

void PeriodicTask::join() {
  if (!m_pImpl->m_taskIsStopped) {
    UniqueLock lock { m_pImpl->m_mutex };
    // wait is not noexcept
    m_pImpl->m_cv.wait(lock, [this]() noexcept { return m_pImpl->m_taskIsStopped.load(); });
  }
}

/*[[nodiscard]]*/ bool PeriodicTask::isStopped() const noexcept { return m_pImpl->m_taskIsStopped; }

void PeriodicTask::start(std::function<bool()> task) {
  m_pImpl->m_taskIsStopped = false;
  m_pImpl->m_futureTaskResult = ThreadPool::submit([this, task]() {
    do {
      UniqueLock lock { m_pImpl->m_mutex };
      const auto& timeout { std::chrono::steady_clock::now() + m_pImpl->m_period };

      // listen to spurious wakes
      while (!m_pImpl->m_stop) { if (std::cv_status::timeout == m_pImpl->m_cv.wait_until(lock, timeout)) { break; } }
    } while (!m_pImpl->m_stop and task());

    m_pImpl->m_taskIsStopped = true;
    m_pImpl->m_cv.notify_one();
  });
}