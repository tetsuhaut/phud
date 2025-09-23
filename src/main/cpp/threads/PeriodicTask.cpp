#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadPool.hpp" // Future, std::atomic_bool

#include <condition_variable>
#include <mutex>

static Logger LOG { CURRENT_FILE_NAME };

struct [[nodiscard]] PeriodicTask::Implementation final {
  Future<void> m_futureTaskResult {};
  std::condition_variable m_cv {};
  std::mutex m_mutex {};
  std::string m_name;
  std::atomic_bool m_stop { false };
  std::atomic_bool m_taskIsStopped { true };
  std::chrono::milliseconds m_period;

  explicit Implementation(std::chrono::milliseconds period, std::string_view taskName)
    : m_name { taskName },
      m_period { period } {}
};

PeriodicTask::PeriodicTask(std::chrono::milliseconds period, std::string_view taskName)
  : m_pImpl { std::make_unique<Implementation>(period, taskName) } {}

PeriodicTask::~PeriodicTask() {
  try {
    stop();
  } catch (...) {
    LOG.error<"Unknown error during the stop of PeriodicTask.">();
  }
}

void PeriodicTask::stop() {
  if (!m_pImpl->m_taskIsStopped) {
    {
      std::unique_lock<std::mutex> lock { m_pImpl->m_mutex }; // noexcept
      m_pImpl->m_futureTaskResult.reset(); // should be noexcept :)
      m_pImpl->m_stop = true;
      m_pImpl->m_cv.notify_all(); // noexcept
    }
    join();
  }
}

void PeriodicTask::join() {
  if (!m_pImpl->m_taskIsStopped) {
    std::unique_lock<std::mutex> lock { m_pImpl->m_mutex };
    // wait is not noexcept
    m_pImpl->m_cv.wait(lock, [this]() noexcept { return m_pImpl->m_taskIsStopped.load(); });
  }
}

/*[[nodiscard]]*/ bool PeriodicTask::isStopped() const noexcept { return m_pImpl->m_taskIsStopped; }
/*[[nodiscard]]*/ bool PeriodicTask::isRunning() const noexcept { return false == m_pImpl->m_taskIsStopped; }

void PeriodicTask::start(std::function<PeriodicTaskStatus()> task) {
  m_pImpl->m_taskIsStopped = false;
  m_pImpl->m_futureTaskResult = ThreadPool::submit([this, task]() {
    do {
      std::unique_lock<std::mutex> lock { m_pImpl->m_mutex };
      const auto& timeout { std::chrono::steady_clock::now() + m_pImpl->m_period };

      // listen to spurious wakes
      while (!m_pImpl->m_stop) { if (std::cv_status::timeout == m_pImpl->m_cv.wait_until(lock, timeout)) { break; } }
    } while (!m_pImpl->m_stop and PeriodicTaskStatus::repeatTask == task());

    m_pImpl->m_taskIsStopped = true;
    m_pImpl->m_cv.notify_one();
  });
}

std::string_view toString(PeriodicTaskStatus status) {
  return PeriodicTaskStatus::repeatTask == status ? "repeatTask" : "stopTask";
}
