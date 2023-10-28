#pragma once

#include "strings/StringUtils.hpp"
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
  TimeBomb(std::chrono::milliseconds countDownToExplosion, std::string_view testName);
  ~TimeBomb();
}; // class TimeBomb