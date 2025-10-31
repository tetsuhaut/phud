#pragma once

#include <chrono>
#include <memory> // std::unique_ptr
#include <string_view>

/**
* This class produces objects that will kill the program after a period of time.
*/
class [[nodiscard]] TimeBomb /*final*/ {
public:
  static std::unique_ptr<TimeBomb> create(std::chrono::milliseconds countDownToExplosion, std::string_view testName);
  virtual ~TimeBomb();
}; // class TimeBomb
