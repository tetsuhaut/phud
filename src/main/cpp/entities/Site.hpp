#pragma once

#include <unordered_map>
#include "strings/StringUtils.hpp" // String, std::string_view
#include "system/memory.hpp" // uptr

class Player;
class CashGame;
class Tournament;

/**
 * A Poker site, i.e. a bunch of hands played on different games, that enables us to build
 * statistics on encountered players behavior.
 */
class [[nodiscard]] Site final {
private:
  std::string m_name;
  std::unordered_map<std::string, uptr<Player>> m_players {};
  std::vector<uptr<CashGame>> m_cashGames {};
  std::vector<uptr<Tournament>> m_tournaments {};

public:
  explicit Site(std::string_view name);
  Site(const Site&) = delete;
  Site(Site&&) = delete;
  Site& operator=(const Site&) = delete;
  Site& operator=(Site&&) = delete;
  ~Site();
  void addPlayer(uptr<Player> p);
  void addGame(uptr<CashGame> cg);
  [[nodiscard]] std::vector<const CashGame*> viewCashGames() const;
  void addGame(uptr<Tournament> t);
  [[nodiscard]] std::vector<const Tournament*> viewTournaments() const;
  [[nodiscard]] /*constexpr*/ std::string getName() const noexcept { return m_name; }
  [[nodiscard]] std::vector<const Player*> viewPlayers() const;
  [[nodiscard]] const Player* viewPlayer(std::string_view name) const {
    const auto& p { m_players.find(std::string(name)) };
    return m_players.end() == p ? nullptr : p->second.get();
  }
  void merge(Site& other);
}; // class Site
