#include "containers/algorithms.hpp" // phud::algorithms
#include "entities/Player.hpp"
#include "language/assert.hpp"
#include "threads/Mutex.hpp"// LockGuard
#include "threads/PlayerCache.hpp"
#include <map>

namespace pa = phud::algorithms;

struct [[nodiscard]] PlayerCache::Implementation final {
  std::map<std::string, uptr<Player>, std::less<>> m_players {};
  Mutex m_mutex {};
  std::string m_siteName;

  Implementation(std::string_view siteName): m_siteName { siteName } {}
};

PlayerCache::PlayerCache(std::string_view siteName) noexcept : m_pImpl { mkUptr<Implementation>(siteName) } {}

PlayerCache::~PlayerCache() = default;

void PlayerCache::setIsHero(std::string_view playerName) {
  const LockGuard lock { m_pImpl->m_mutex };
  auto it { m_pImpl->m_players.find(playerName) };
  phudAssert(m_pImpl->m_players.end() != it, "Setting hero on a bad player");
  it->second->setIsHero(true);
}

void PlayerCache::erase(std::string_view playerName) {
  const LockGuard lock { m_pImpl->m_mutex };
  auto it { m_pImpl->m_players.find(playerName) };
  phudAssert(m_pImpl->m_players.end() != it, "Erasing a bad player");
  m_pImpl->m_players.erase(it);
}

void PlayerCache::addIfMissing(std::string_view playerName) {
  const LockGuard lock { m_pImpl->m_mutex };
  m_pImpl->m_players.emplace(std::make_pair(playerName, mkUptr<Player>(Player::Params{ .name = playerName, .site = m_pImpl->m_siteName })));
}

bool PlayerCache::isEmpty() {
  const LockGuard lock { m_pImpl->m_mutex };
  return m_pImpl->m_players.empty();
}

std::vector<uptr<Player>> PlayerCache::extractPlayers() {
  const LockGuard lock { m_pImpl->m_mutex };
  std::vector<uptr<Player>> ret;
  ret.reserve(m_pImpl->m_players.size());
  pa::forEach(m_pImpl->m_players, [&](auto & nameToPlayer) { ret.push_back(std::move(nameToPlayer.second)); });
  m_pImpl->m_players.clear();
  return ret;
}