#pragma once

#include <memory> // std::unique_ptr, std::is_same_v
#include <utility> // std::pair

// forward declarations
struct GameData;
class Hand;
class PlayerCache;
class TextFile;

namespace WinamaxHandBuilder {
  [[nodiscard]] std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>>
  buildCashgameHandAndGameData(TextFile& tf,
                               PlayerCache& pc);

  [[nodiscard]] std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>>
  buildTournamentHandAndGameData(
    TextFile& tf, PlayerCache& pc);

  template <typename GAME_TYPE>
    requires(std::is_same_v<GAME_TYPE, CashGame>
      or std::is_same_v<GAME_TYPE, Tournament>)
  [[nodiscard]] std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>> buildHandAndGameData(
    TextFile& tf,
    PlayerCache& pc) {
    if constexpr (std::is_same_v<GAME_TYPE, CashGame>) { return buildCashgameHandAndGameData(tf, pc); }
    if constexpr (std::is_same_v < GAME_TYPE, Tournament>) { return buildTournamentHandAndGameData(tf, pc); }
  }

  [[nodiscard]] std::unique_ptr<Hand> buildCashgameHand(TextFile& tf, PlayerCache& pc);

  [[nodiscard]] std::unique_ptr<Hand> buildTournamentHand(TextFile& tf, PlayerCache& pc);

  template <typename GAME_TYPE>
    requires(std::is_same_v<GAME_TYPE, CashGame>
      or std::is_same_v<GAME_TYPE, Tournament>)
  [[nodiscard]] std::unique_ptr<Hand> buildHand(TextFile& tf, PlayerCache& pc) {
    if constexpr (std::is_same_v<GAME_TYPE, CashGame>) { return buildCashgameHand(tf, pc); }
    if constexpr (std::is_same_v<GAME_TYPE, Tournament>) { return buildTournamentHand(tf, pc); }
  }
} // namespace WinamaxHandBuilder
