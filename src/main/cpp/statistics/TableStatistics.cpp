#include "containers/algorithms.hpp"
#include "language/assert.hpp"
#include "language/limits.hpp"
#include "statistics/PlayerStatistics.hpp" // toSizeT
#include "statistics/TableStatistics.hpp"
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;


Seat TableStatistics::getHeroSeat() const {
  const auto it { pa::findIf(m_tableStats, [this](const auto & playerStat) noexcept { return playerStat and playerStat->isHero(); }) };
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

/*[[nodiscard]]*/ std::vector<Seat> TableStatistics::getSeats() {
  return pa::getValueFromKey(MAX_SEAT_TO_TABLE_SEATS, getMaxSeat());
}