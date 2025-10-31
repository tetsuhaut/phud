#include "TimeBomb.hpp"
#include "strings/StringUtils.hpp"
#include "threads/PeriodicTask.hpp"

#include <print>
#include <sstream> // std::ostringstream
#include <thread>

TimeBomb::~TimeBomb() = default;

struct [[nodiscard]] TimeBombImpl final : TimeBomb {
  PeriodicTask m_task;
  std::string m_testName;
  std::atomic_bool m_isDefused { false };

  TimeBombImpl(std::chrono::milliseconds countDownToExplosion, std::string_view testName)
    : m_task { countDownToExplosion, "TimeBomb" },
      m_testName { testName } {
    m_task.start([this]() {
      if (!m_isDefused) {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::print("[{}] TimeBomb explodes in test {}\n", oss.str(), m_testName);
        std::abort();
      }

      return PeriodicTaskStatus::stopTask;
    });
  }

  ~TimeBombImpl() {
    m_isDefused = true;
    m_task.stop();
  }
};

std::unique_ptr<TimeBomb> TimeBomb::create(std::chrono::milliseconds countDownToExplosion, std::string_view testName) {
  return std::make_unique<TimeBombImpl>(countDownToExplosion, testName);
}
