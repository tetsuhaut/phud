#pragma once

#include "strings/StringUtils.hpp"

#include <memory> // std::unique_ptr, std::is_same_v
#include <utility> // std::pair

struct GameData;
class Hand;
class PlayerCache;
class TextFile;

namespace WinamaxHandBuilder {
[[nodiscard]] std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>> buildCashgameHandAndGameData(TextFile& tfl,
    PlayerCache& pc);

[[nodiscard]] std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>> buildTournamentHandAndGameData(
      TextFile& tfl, PlayerCache& pc);

template<typename GAME_TYPE>
requires(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>)
[[nodiscard]] std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>> buildHandAndGameData(TextFile& tfl,
PlayerCache& pc) {
  if constexpr(std::is_same_v<GAME_TYPE, CashGame>) { return buildCashgameHandAndGameData(tfl, pc); }

  if constexpr(std::is_same_v<GAME_TYPE, Tournament>) { return buildTournamentHandAndGameData(tfl, pc); }
}

[[nodiscard]] std::unique_ptr<Hand> buildCashgameHand(TextFile& tfl, PlayerCache& pc);

[[nodiscard]] std::unique_ptr<Hand> buildTournamentHand(TextFile& tfl, PlayerCache& pc);

template<typename GAME_TYPE>
requires(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>)
[[nodiscard]] std::unique_ptr<Hand> buildHand(TextFile& tfl, PlayerCache& pc) {
  if constexpr(std::is_same_v<GAME_TYPE, CashGame>) { return buildCashgameHand(tfl, pc); }

  if constexpr(std::is_same_v<GAME_TYPE, Tournament>) { return buildTournamentHand(tfl, pc); }
}
}; // WinamaxHandBuilder
