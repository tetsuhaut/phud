#include "containers/algorithms.hpp" // phud::algorithms::*
#include "entities/Game.hpp" // CashGame, Tournament
#include "entities/Player.hpp"
#include "entities/Site.hpp" // Site
#include "language/assert.hpp" // phudAssert

namespace pa = phud::algorithms;

Site::Site(std::string_view name)
  : m_name { name } { phudAssert(!m_name.empty(), "name is empty"); }

Site::~Site() = default; // needed because Site owns private uptr members

std::vector<const Player*> Site::viewPlayers() const {
  std::vector<const Player*> ret;
  if (false == m_players.empty()) {
    ret.reserve(m_players.size());
    pa::transform(m_players, ret, [](const auto& entry) noexcept { return entry.second.get(); });
  }
  return ret;
}

void Site::addPlayer(uptr<Player> p) {
  phudAssert(p->getSiteName() == m_name, "player is on another site");

  if (!m_players.contains(p->getName())) {
    m_players[p->getName()] = std::move(p);
  }
}

void Site::addGame(uptr<CashGame> game) {
  phudAssert(game->getSiteName() == m_name, "game is on another site");
  m_cashGames.push_back(std::move(game));
}

void Site::addGame(uptr<Tournament> game) {
  phudAssert(game->getSiteName() == m_name, "game is on another site");
  m_tournaments.push_back(std::move(game));
}

std::vector<const CashGame*> Site::viewCashGames() const { return pa::mkView(m_cashGames); }

std::vector<const Tournament*> Site::viewTournaments() const { return pa::mkView(m_tournaments); }

void Site::merge(Site& other) {
  phudAssert(other.getName() == m_name, "Can't merge data from different poker sites");
  pa::forEach(other.m_players, [this](auto & pair) { addPlayer(std::move(pair.second)); });
  pa::moveInto(other.m_cashGames, m_cashGames);
  pa::moveInto(other.m_tournaments, m_tournaments);
}