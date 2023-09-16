#include "TimeBomb.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/Thread.hpp" // getCurrentThreadId
#include <spdlog/fmt/bundled/printf.h> // fmt::print

struct [[nodiscard]] TimeBomb::Implementation final {
  PeriodicTask m_task;
  String m_testName;
  std::atomic_bool m_isDefused { false };

  Implementation(std::chrono::milliseconds countDownToExplosion, StringView testName)
    : m_task { countDownToExplosion, "TimeBomb" },
      m_testName { testName } {}
};

TimeBomb::~TimeBomb() {
  m_pImpl->m_isDefused = true;
  m_pImpl->m_task.stop();
}

TimeBomb::TimeBomb(std::chrono::milliseconds countDownToExplosion, StringView testName)
  : m_pImpl { mkUptr<Implementation>(countDownToExplosion, testName) } {
  m_pImpl->m_task.start([this]() {
    if (!m_pImpl->m_isDefused) {
      fmt::print("[{}] TimeBomb explodes in test {}\n", getCurrentThreadId(), m_pImpl->m_testName);
      std::terminate();
    }

    return false;
  });
}
