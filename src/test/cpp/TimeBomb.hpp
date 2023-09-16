#pragma once

#include "strings/StringView.hpp"
#include "system/memory.hpp" // uptr
#include <chrono>

/**
* This class produces objects that will kill the program after a period of time.
*/
class [[nodiscard]] TimeBomb final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  TimeBomb(std::chrono::milliseconds countDownToExplosion, StringView testName);
  ~TimeBomb();
}; // class TimeBomb