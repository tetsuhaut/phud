#pragma once

#include "entities/Game.hpp" // std::string, std::string_view, Time, Variant, Limit
#include "entities/Seat.hpp"

struct [[nodiscard]] GameData final {
  Seat m_nbMaxSeats { Seat::seatUnknown };
  bool m_isRealMoney { false };
  Variant m_variant { Variant::none };
  Limit m_limit { Limit::none };
  double m_smallBlind { 0 };
  double m_bigBlind { 0 };
  double m_buyIn { 0 };
  std::string m_tableName {};
  std::string m_gameName {};
  Time m_startDate;

  struct [[nodiscard]] Args final {
    Seat nbMaxSeats;
    double smallBlind;
    double bigBlind;
    double buyIn;
    const Time& startDate;
  };

  explicit GameData(const Args& args) noexcept
    : m_nbMaxSeats { args.nbMaxSeats },
      m_smallBlind { args.smallBlind },
      m_bigBlind { args.bigBlind },
      m_buyIn { args.buyIn },
      m_startDate { args.startDate } {}
}; // struct GameData
