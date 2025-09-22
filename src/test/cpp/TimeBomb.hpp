#pragma once

#include <chrono>
#include <memory> // std::unique_ptr
#include <string_view>

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