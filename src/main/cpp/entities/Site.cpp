#include "entities/Game.hpp" // CashGame, Tournament
#include "entities/Site.hpp" // Site, Player
#include "language/Validator.hpp"
#include <algorithm> // std::move, also needed for std::ranges::transform, std::ranges::for_each
#include <iterator>  // std::back_inserter
#include <span>      // std::span

Site::Site(std::string_view name)
  : m_name {name} {
  validation::requireNonEmpty(m_name, "name");
}

Site::~Site() = default; // needed because Site owns private std::unique_ptr members

std::vector<const Player*> Site::viewPlayers() const {
  std::vector<const Player*> ret;

  if (false == m_players.empty()) {
    ret.reserve(m_players.size());
    std::ranges::transform(m_players, std::back_inserter(ret),
                           [](const auto& entry) noexcept { return entry.second.get(); });
  }

  return ret;
}

void Site::addPlayer(std::unique_ptr<Player> p) {
  validation::require(p->getSiteName() == m_name, "player is on another site");

  // Use try_emplace to avoid double lookup (contains + operator[])
  // try_emplace only inserts if the key doesn't exist, avoiding the contains() call
  m_players.try_emplace(p->getName(), std::move(p));
}

void Site::addGame(std::unique_ptr<CashGame> cg) {
  validation::require(cg->getSiteName() == m_name, "game is on another site");
  m_cashGames.push_back(std::move(cg));
}

void Site::addGame(std::unique_ptr<Tournament> t) {
  validation::require(t->getSiteName() == m_name, "game is on another site");
  m_tournaments.push_back(std::move(t));
}

template <typename T>
static std::vector<const T*> view(std::span<const std::unique_ptr<T>> source) {
  std::vector<const T*> ret;
  ret.reserve(source.size());
  std::ranges::transform(source, std::back_inserter(ret),
                         [](const auto& pointer) { return pointer.get(); });
  return ret;
}

std::vector<const CashGame*> Site::viewCashGames() const {
  return view(std::span(m_cashGames));
}

std::vector<const Tournament*> Site::viewTournaments() const {
  return view(std::span(m_tournaments));
}

void Site::merge(Site& other) {
  validation::require(other.getName() == m_name, "Can't merge data from different poker sites");

  // Pre-allocate space for players to reduce rehashing
  // Reserve enough space for current + new players (assuming some duplicates)
  if (0 == m_players.size()) {
    // First merge: reserve exact size
    m_players.reserve(other.m_players.size());
  } else if (!other.m_players.empty()) {
    // Subsequent merges: reserve for worst case (no duplicates)
    m_players.reserve(m_players.size() + other.m_players.size());
  }

  // Merge players using try_emplace for efficiency
  std::ranges::for_each(other.m_players, [this](auto& pair) { addPlayer(std::move(pair.second)); });

  // Pre-allocate space for games
  m_cashGames.reserve(m_cashGames.size() + other.m_cashGames.size());
  m_tournaments.reserve(m_tournaments.size() + other.m_tournaments.size());

  std::ranges::move(other.m_cashGames, std::back_inserter(m_cashGames));
  std::ranges::move(other.m_tournaments, std::back_inserter(m_tournaments));
}
