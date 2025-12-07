#pragma once

#include <string>
#include <string_view>

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
  // Memory layout optimized: largest to smallest to minimize padding
  std::string m_handId;
  std::string m_playerName;
  std::size_t m_index; // 8 bytes
  double m_betAmount;  // 8 bytes
  Street m_street;     // 2 bytes (short)
  ActionType m_type;   // 2 bytes (short)
public:
  struct [[nodiscard]] Params final {
    std::string_view handId;
    std::string_view playerName;
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
  [[nodiscard]] constexpr Street getStreet() const noexcept { return m_street; }
  [[nodiscard]] constexpr const std::string& getHandId() const noexcept { return m_handId; }
  [[nodiscard]] constexpr const std::string& getPlayerName() const noexcept { return m_playerName; }
  [[nodiscard]] constexpr ActionType getType() const noexcept { return m_type; }
  [[nodiscard]] constexpr std::size_t getIndex() const noexcept { return m_index; }
  [[nodiscard]] constexpr double getBetAmount() const noexcept { return m_betAmount; }
}; // class Action

// exported methods
[[nodiscard]] std::string_view toString(ActionType at);
[[nodiscard]] std::string_view toString(Street st);
