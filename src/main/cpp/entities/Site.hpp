#pragma once

#include "containers/Map.hpp"
#include "containers/Vector.hpp" // Vector
#include "strings/String.hpp" // String, StringView
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
  String m_name;
  Map<String, uptr<Player>> m_players {};
  Vector<uptr<CashGame>> m_cashGames {};
  Vector<uptr<Tournament>> m_tournaments {};

public:
  explicit Site(StringView name);
  Site(const Site&) = delete;
  Site(Site&&) = delete;
  Site& operator=(const Site&) = delete;
  Site& operator=(Site&&) = delete;
  ~Site();
  void addPlayer(uptr<Player> p);
  void addGame(uptr<CashGame> cg);
  [[nodiscard]] Vector<const CashGame*> viewCashGames() const;
  void addGame(uptr<Tournament> t);
  [[nodiscard]] Vector<const Tournament*> viewTournaments() const;
  [[nodiscard]] /*constexpr*/ String getName() const noexcept { return m_name; }
  [[nodiscard]] Vector<const Player*> viewPlayers() const;
  [[nodiscard]] const Player* viewPlayer(StringView name) const {
    const auto& p { m_players.find(String(name)) };
    return m_players.end() == p ? nullptr : p->second.get();
  }
  void merge(Site& other);
}; // class Site
