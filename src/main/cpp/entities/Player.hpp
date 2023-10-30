#pragma once

#include <string>
#include <string_view>

/**
 * A tournament or cashgame poker player.
 */
class [[nodiscard]] Player final {
private:
  std::string m_name;
  std::string m_site;
  std::string m_comments;
  bool m_isHero = false;

public:
  struct [[nodiscard]] Params final {
    std::string_view name;
    std::string_view site;
    std::string_view comments { "" };
  };

  explicit Player(const Params& p);
  Player(const Player&) = delete;
  Player(Player&&) = delete;
  Player& operator=(const Player&) = delete;
  Player& operator=(Player&&) = delete;
  ~Player();
  // gcc 10.2 can't do constexpr std::string
  [[nodiscard]] /*constexpr*/ std::string getName() const noexcept { return m_name; }
  [[nodiscard]] /*constexpr*/ std::string getPlayerName() const noexcept { return m_name; }
  [[nodiscard]] /*constexpr*/ std::string getSiteName() const noexcept { return m_site; }
  [[nodiscard]] /*constexpr*/ std::string getComments() const noexcept { return m_comments; }
  [[nodiscard]] constexpr bool isHero() const noexcept { return m_isHero; }
  constexpr void setIsHero(bool isHero) noexcept { m_isHero = isHero; }
}; // class Player
