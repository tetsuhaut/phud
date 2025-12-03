#pragma once

#include "entities/Game.hpp" // std::string, std::string_view, Time, Variant, Limit
#include "entities/Seat.hpp"

struct [[nodiscard]] GameData final {
  // Memory layout optimized: largest to smallest to minimize padding
  std::string m_tableName {};
  std::string m_gameName {};
  Time m_startDate;                      // 8 bytes (std::unique_ptr)
  double m_smallBlind {0};               // 8 bytes
  double m_bigBlind {0};                 // 8 bytes
  double m_buyIn {0};                    // 8 bytes
  Seat m_nbMaxSeats {Seat::seatUnknown}; // 2 bytes (short)
  Variant m_variant {Variant::none};     // 2 bytes (short)
  Limit m_limit {Limit::none};           // 2 bytes (short)
  bool m_isRealMoney {false};            // 1 byte
  // 1 byte padding at end

  struct [[nodiscard]] Args final {
    double smallBlind;
    double bigBlind;
    double buyIn;
    Seat nbMaxSeats;
    const Time& startDate;
  };

  explicit GameData(const Args& args) noexcept
    : m_startDate {args.startDate},
      m_smallBlind {args.smallBlind},
      m_bigBlind {args.bigBlind},
      m_buyIn {args.buyIn},
      m_nbMaxSeats {args.nbMaxSeats} {}
}; // struct GameData
