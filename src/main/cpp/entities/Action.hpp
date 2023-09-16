#pragma once

#include "strings/String.hpp" // StringView

/**
 * The elementary move a player can do during the game.
 */
enum /*[[nodiscard]]*/ class ActionType : short { none, fold, check, call, bet, raise };

/**
 * The name of the current game board configuration.
 */
enum class /*[[nodiscard]]*/ Street : short { none, preflop, flop, turn, river };

/**
 * The elementary move of a player.
 */
class [[nodiscard]] Action final {
private:
  String m_handId;
  String m_playerName;
  Street m_street;
  ActionType m_type;
  std::size_t m_index;
  double m_betAmount;

public:

  struct [[nodiscard]] Params final {
    StringView handId;
    StringView playerName;
    Street street;
    ActionType type;
    std::size_t actionIndex;
    double betAmount;
  };

  explicit Action(const Params& p);
  Action(const Action&) = delete;
  Action(Action&&) = delete;
  Action& operator=(const Action&) = delete;
  Action& operator=(Action&&) = delete;
  ~Action();
  [[nodiscard]] Street getStreet() const noexcept { return m_street; }
  [[nodiscard]] String getHandId() const noexcept { return m_handId; }
  [[nodiscard]] String getPlayerName() const noexcept { return m_playerName; }
  [[nodiscard]] ActionType getType() const noexcept { return m_type; }
  [[nodiscard]] std::size_t getIndex() const noexcept { return m_index; }
  [[nodiscard]] double getBetAmount() const noexcept { return m_betAmount; }
}; // class Action

// exported methods
[[nodiscard]] StringView toString(ActionType at);
[[nodiscard]] StringView toString(Street st);
