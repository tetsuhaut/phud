#include "entities/Game.hpp" // CashGame, Tournament
#include "entities/Site.hpp" // Site, Player
#include "language/Validator.hpp"
#include <algorithm> // std::move, also needed for std::ranges::transform, std::ranges::for_each
#include <iterator> // std::back_inserter
#include <ranges> // std::ranges::transform, std::ranges::for_each

Site::Site(std::string_view name)
  : m_name { name } { validation::requireNonEmpty(m_name, "name"); }

Site::~Site() = default; // needed because Site owns private std::unique_ptr members

std::vector<const Player*> Site::viewPlayers() const {
  std::vector<const Player*> ret;

  if (false == m_players.empty()) {
    ret.reserve(m_players.size());
    std::ranges::transform(m_players, std::back_inserter(ret),
    [](const auto & entry) noexcept { return entry.second.get(); });
  }

  return ret;
}

void Site::addPlayer(std::unique_ptr<Player> p) {
  validation::require(p->getSiteName() == m_name, "player is on another site");

  if (!m_players.contains(p->getName())) {
    m_players[p->getName()] = std::move(p);
  }
}

void Site::addGame(std::unique_ptr<CashGame> game) {
  validation::require(game->getSiteName() == m_name, "game is on another site");
  m_cashGames.push_back(std::move(game));
}

void Site::addGame(std::unique_ptr<Tournament> game) {
  validation::require(game->getSiteName() == m_name, "game is on another site");
  m_tournaments.push_back(std::move(game));
}

template<typename T>
static std::vector<const T*> view(const std::vector<std::unique_ptr<T>>& source) {
  std::vector<const T*> ret;
  ret.reserve(source.size());
  std::ranges::transform(source, std::back_inserter(ret), [](const auto& pointer) { return pointer.get(); });
  return ret;
}

std::vector<const CashGame*> Site::viewCashGames() const { return view(m_cashGames); }

std::vector<const Tournament*> Site::viewTournaments() const { return view(m_tournaments); }

void Site::merge(Site& other) {
  validation::require(other.getName() == m_name, "Can't merge data from different poker sites");
  std::ranges::for_each(other.m_players, [this](auto & pair) { addPlayer(std::move(pair.second)); });
  std::move(std::begin(other.m_cashGames), std::end(other.m_cashGames),
            std::back_inserter(m_cashGames));
  std::move(std::begin(other.m_tournaments), std::end(other.m_tournaments),
            std::back_inserter(m_tournaments));
}
