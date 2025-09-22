#include "language/limits.hpp"
#include "statistics/PlayerStatistics.hpp" // toSizeT
#include "statistics/TableStatistics.hpp"
#include <frozen/unordered_map.h>

#include <ranges>

static bool isHero(const std::unique_ptr<PlayerStatistics>& pPlayerStat) noexcept {
  return pPlayerStat and pPlayerStat->isHero();
}

// as TableStatistics owns std::unique_ptr
TableStatistics::~TableStatistics() = default;

Seat TableStatistics::getHeroSeat() const {
  const auto it { std::ranges::find_if(m_tableStats, isHero) };
  return (m_tableStats.end() == it) ? Seat::seatUnknown : tableSeat::fromArrayIndex(
           limits::toSizeT(it - m_tableStats.begin()));
}

std::unique_ptr<PlayerStatistics> TableStatistics::extractPlayerStatistics(Seat seat) {
  return std::exchange(m_tableStats.at(tableSeat::toArrayIndex(seat)), nullptr);
}

static const auto MAX_SEAT_TO_TABLE_SEATS {
  frozen::make_unordered_map<Seat, std::vector<Seat>>({
    {Seat::seatTwo, {Seat::seatOne, Seat::seatTwo}},
    {Seat::seatThree, {Seat::seatOne, Seat::seatTwo, Seat::seatThree}},
    {Seat::seatFour, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour}},
    {Seat::seatFive, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive}},
    {Seat::seatSix, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix}},
    {Seat::seatSeven, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix, Seat::seatSeven}},
    {Seat::seatEight, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix, Seat::seatSeven, Seat::seatEight}},
    {Seat::seatNine, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix, Seat::seatSeven, Seat::seatEight, Seat::seatNine}},
    {Seat::seatTen, {Seat::seatOne, Seat::seatTwo, Seat::seatThree, Seat::seatFour, Seat::seatFive, Seat::seatSix, Seat::seatSeven, Seat::seatEight, Seat::seatNine, Seat::seatTen}},
  })
};

/*[[nodiscard]]*/ std::vector<Seat> TableStatistics::getSeats() const {
  const auto seat { getMaxSeat() };
  return MAX_SEAT_TO_TABLE_SEATS.find(seat)->second;
}