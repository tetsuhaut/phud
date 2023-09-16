#pragma once

#include "strings/String.hpp" // String, StringView

/**
 * A tournament or cashgame poker player.
 */
class [[nodiscard]] Player final {
private:
  String m_name;
  String m_site;
  String m_comments;
  bool m_isHero = false;

public:
  struct [[nodiscard]] Params final {
    StringView name;
    StringView site;
    StringView comments { "" };
  };

  explicit Player(const Params& p);
  Player(const Player&) = delete;
  Player(Player&&) = delete;
  Player& operator=(const Player&) = delete;
  Player& operator=(Player&&) = delete;
  ~Player();
  // gcc 10.2 can't do constexpr std::string
  [[nodiscard]] /*constexpr*/ String getName() const noexcept { return m_name; }
  [[nodiscard]] /*constexpr*/ String getPlayerName() const noexcept { return m_name; }
  [[nodiscard]] /*constexpr*/ String getSiteName() const noexcept { return m_site; }
  [[nodiscard]] /*constexpr*/ String getComments() const noexcept { return m_comments; }
  [[nodiscard]] constexpr bool isHero() const noexcept { return m_isHero; }
  constexpr void setIsHero(bool isHero) noexcept { m_isHero = isHero; }
}; // class Player
