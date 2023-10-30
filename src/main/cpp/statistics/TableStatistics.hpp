#pragma once

#include "entities/Seat.hpp"

#include <array>
#include <memory> // std::unique_ptr

class PlayerStatistics;

struct [[nodiscard]] TableStatistics final {
  Seat m_maxSeats { Seat::seatUnknown };
  std::array<std::unique_ptr<PlayerStatistics>, 10> m_tableStats {};

  [[nodiscard]] constexpr bool isValid() const noexcept { return Seat::seatUnknown != m_maxSeats; }

  [[nodiscard]] constexpr Seat getMaxSeat() const noexcept { return m_maxSeats; }

  [[nodiscard]] std::vector<Seat> getSeats();

  /**
   * @returns the hero seat. 0 if error, else between 1 and 10 included.
   */
  [[nodiscard]] Seat getHeroSeat() const;

  /**
  * @returns the statistics for the player at the given seat.
  * If the seat is empty, the returned pointer is nullptr.
  * 0 < seat < 11
  */
  [[nodiscard]] std::unique_ptr<PlayerStatistics> extractPlayerStatistics(Seat seat);
}; // struct TableStatistics