#pragma once

#include "strings/StringUtils.hpp"

#include <chrono>
#include <memory> // std::unique_ptr

/**
* This class produces objects that will kill the program after a period of time.
*/
class [[nodiscard]] TimeBomb final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  TimeBomb(std::chrono::milliseconds countDownToExplosion, std::string_view testName);
  ~TimeBomb();
}; // class TimeBomb