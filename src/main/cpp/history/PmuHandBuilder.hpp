#pragma once

#include "containers/Pair.hpp"
#include "strings/StringView.hpp"
#include "system/memory.hpp" // uptr, std::is_same_v

struct GameData;
class Hand;
class PlayerCache;
class TextFile;

namespace PmuHandBuilder {
[[nodiscard]] Pair<uptr<Hand>, uptr<GameData>> buildCashgameHandAndGameData(TextFile& tfl,
    PlayerCache& pc);
[[nodiscard]] Pair<uptr<Hand>, uptr<GameData>> buildTournamentHandAndGameData(
      TextFile& tfl, PlayerCache& pc);
template<typename GAME_TYPE>
[[nodiscard]] Pair<uptr<Hand>, uptr<GameData>> buildHandAndGameData(TextFile& tfl,
PlayerCache& pc) {
  static_assert(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>);

  if constexpr(std::is_same_v<GAME_TYPE, CashGame>) { return buildCashgameHandAndGameData(tfl, pc); }

  if constexpr(std::is_same_v<GAME_TYPE, Tournament>) { return buildTournamentHandAndGameData(tfl, pc); }
}
[[nodiscard]] uptr<Hand> buildCashgameHand(TextFile& tfl, PlayerCache& pc);
[[nodiscard]] uptr<Hand> buildTournamentHand(TextFile& tfl, PlayerCache& pc);
template<typename GAME_TYPE>
[[nodiscard]] uptr<Hand> buildHand(TextFile& tfl, PlayerCache& pc) {
  static_assert(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>);

  if constexpr(std::is_same_v<GAME_TYPE, CashGame>) { return buildCashgameHand(tfl, pc); }

  if constexpr(std::is_same_v<GAME_TYPE, Tournament>) { return buildTournamentHand(tfl, pc); }
}
}; // PmuHandBuilder