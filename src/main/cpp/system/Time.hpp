#pragma once

#include "language/PhudException.hpp" // PhudException, std::string_view
#include <memory>                     // std::unique_ptr

// forward declaration
namespace std {
using ::tm; // NOLINT(*-dcl58-cpp)
}

class [[nodiscard]] Time final { /* copyable */
 private:
  std::unique_ptr<std::tm> m_pTimeData;

 public:
  struct [[nodiscard]] Args final {
    std::string_view strTime;
    std::string_view format;
  };

  explicit Time(const Args& args);
  Time(const Time&) noexcept;
  Time(Time&&) noexcept;
  Time& operator=(const Time&) noexcept;
  Time& operator=(Time&&) noexcept;
  [[nodiscard]] bool operator==(const Time&) const noexcept;
  [[nodiscard]] std::string toSqliteDate() const;
}; // class Time

class [[nodiscard]] TimeException final : public PhudException {
 public:
  using PhudException::PhudException;
};

static constexpr std::string_view WINAMAX_HISTORY_TIME_FORMAT {
    "%Y/%m/%d %H:%M:%S"}; // ex: 2014/10/31 00:45:01
// static constexpr std::string_view PMU_HISTORY_TIME_FORMAT{ "%A, %B %d, %H:%M:%S %Z %Y" }; // ex:
// Tuesday, September 14, 18:33:39 CEST 2021
static constexpr std::string_view PMU_HISTORY_TIME_FORMAT {"%A, %B %d, %H:%M:%S %Y"};
// ex: Tuesday, September 14, 18:33:39 2021
