#pragma once

#include "language/PhudException.hpp" // PhudException, StringView
#include "strings/String.hpp"
#include "system/memory.hpp" // uptr

// forward declaration
namespace std { using ::tm; };

class [[nodiscard]] Time final { /* copyable */
private:
  uptr<std::tm> m_pTimeData;

public:
  struct [[nodiscard]] Args final { StringView strTime; StringView format; };
  explicit Time(const Args& args);
  Time(const Time& t) noexcept;
  explicit Time(Time&&) noexcept;
  Time& operator=(const Time&) noexcept;
  Time& operator=(Time&&) noexcept;
  [[nodiscard]] bool operator==(const Time& other) const noexcept;
  [[nodiscard]] String toSqliteDate() const;
}; // class Time

class [[nodiscard]] TimeException : public PhudException {
public:
  using PhudException::PhudException;
};

static constexpr StringView WINAMAX_HISTORY_TIME_FORMAT { "%Y/%m/%d %H:%M:%S" }; // ex: 2014/10/31 00:45:01
//static constexpr StringView PMU_HISTORY_TIME_FORMAT{ "%A, %B %d, %H:%M:%S %Z %Y" }; // ex: Tuesday, September 14, 18:33:39 CEST 2021
static constexpr StringView PMU_HISTORY_TIME_FORMAT { "%A, %B %d, %H:%M:%S %Y" }; // ex: Tuesday, September 14, 18:33:39 2021