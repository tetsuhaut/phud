#include "TimeBomb.hpp"
#include "threads/PeriodicTask.hpp"
#include <spdlog/fmt/bundled/printf.h> // fmt::print

#include <sstream> // std::ostringstream
#include <thread>

struct [[nodiscard]] TimeBomb::Implementation final {
  PeriodicTask m_task;
  std::string m_testName;
  std::atomic_bool m_isDefused { false };

  Implementation(std::chrono::milliseconds countDownToExplosion, std::string_view testName)
    : m_task { countDownToExplosion, "TimeBomb" },
      m_testName { testName } {}
};

TimeBomb::~TimeBomb() {
  m_pImpl->m_isDefused = true;
  m_pImpl->m_task.stop();
}

TimeBomb::TimeBomb(std::chrono::milliseconds countDownToExplosion, std::string_view testName)
  : m_pImpl { std::make_unique<Implementation>(countDownToExplosion, testName) } {
  m_pImpl->m_task.start([this]() {
    if (!m_pImpl->m_isDefused) {
      std::ostringstream oss;
      oss << std::this_thread::get_id();
      fmt::print("[{}] TimeBomb explodes in test {}\n", oss.str(), m_pImpl->m_testName);
      std::terminate();
    }

    return PeriodicTaskStatus::stopTask;
  });
}
