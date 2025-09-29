#pragma once

#include "constants/TableConstants.hpp"
#include "entities/Seat.hpp"

#include <array>
#include <memory> // std::unique_ptr
#include <vector>

class PlayerStatistics;

struct [[nodiscard]] TableStatistics final {
  std::string m_site {};
  std::string m_table {};
  Seat m_maxSeats { Seat::seatUnknown };
  std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS> m_tableStats {};
  TableStatistics() = default;

  TableStatistics(std::string_view site, std::string_view table, Seat maxSeats,
                  std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS> tableStats)
    : m_site { site },
      m_table { table },
      m_maxSeats { maxSeats },
      m_tableStats { std::move(tableStats) } {}

  TableStatistics(const TableStatistics&) = delete;
  TableStatistics(TableStatistics&&) = default;
  TableStatistics& operator=(const TableStatistics&) = delete;
  TableStatistics& operator=(TableStatistics&&) = default;
  ~TableStatistics();

  [[nodiscard]] constexpr const std::string& getSite() { return m_site; }
  [[nodiscard]] constexpr const std::string& getTable() { return m_table; }

  [[nodiscard]] constexpr bool isValid() const noexcept { return Seat::seatUnknown != m_maxSeats; }

  [[nodiscard]] constexpr Seat getMaxSeat() const noexcept { return m_maxSeats; }

  [[nodiscard]] std::vector<Seat> getSeats() const;

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
