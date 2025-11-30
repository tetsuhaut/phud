#include "statistics/PlayerStatistics.hpp" // toSizeT
#include "statistics/TableStatistics.hpp"
#include <ranges>

namespace {
  bool isHero(const std::unique_ptr<PlayerStatistics>& pPlayerStat) noexcept {
    return pPlayerStat and pPlayerStat->isHero();
  }

  std::vector<Seat> getSeats(Seat maxSeat) {
    static constexpr std::array<Seat, 10> allSeats {
      Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive,
      Seat::seatSix, Seat::seatSeven, Seat::seatEight, Seat::seatNine, Seat::seatTen
    };

    const auto count { tableSeat::toInt(maxSeat) };
    if (0 >= count || 10 < count) {
      return {};
    }
    return std::vector<Seat>(allSeats.begin(), allSeats.begin() + count);
  }
} // anonymous namespace

// as TableStatistics owns std::unique_ptr
TableStatistics::~TableStatistics() = default;

TableStatistics::TableStatistics(std::string_view site, std::string_view table, Seat maxSeats,
  std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS> tableStats)
  : m_site { site },
  m_table { table },
  m_maxSeats { maxSeats },
  m_seats { ::getSeats(maxSeats) },
  m_tableStats { std::move(tableStats) } {
}

TableStatistics::TableStatistics() = default;

Seat TableStatistics::getHeroSeat() const {
  const auto it { std::ranges::find_if(m_tableStats, isHero) };
  return (m_tableStats.end() == it) ? Seat::seatUnknown : tableSeat::fromArrayIndex(
           limits::toSizeT(it - m_tableStats.begin()));
}

std::unique_ptr<PlayerStatistics> TableStatistics::extractPlayerStatistics(Seat seat) {
  return std::exchange(m_tableStats.at(tableSeat::toArrayIndex(seat)), nullptr);
}
