#include "containers/algorithms.hpp" // phud::algorithms
#include "entities/Player.hpp"
#include "language/assert.hpp"
#include "threads/Mutex.hpp"// LockGuard
#include "threads/PlayerCache.hpp"
#include <map>

namespace pa = phud::algorithms;

struct [[nodiscard]] PlayerCache::Implementation final {
  std::map<String, uptr<Player>, std::less<>> m_players {};
  Mutex m_mutex {};
  String m_siteName;

  Implementation(StringView siteName): m_siteName { siteName } {}
};

PlayerCache::PlayerCache(StringView siteName) noexcept : m_pImpl { mkUptr<Implementation>(siteName) } {}

PlayerCache::~PlayerCache() = default;

void PlayerCache::setIsHero(StringView playerName) {
  const LockGuard lock { m_pImpl->m_mutex };
  auto it { m_pImpl->m_players.find(playerName) };
  phudAssert(m_pImpl->m_players.end() != it, "Setting hero on a bad player");
  it->second->setIsHero(true);
}

void PlayerCache::erase(StringView playerName) {
  const LockGuard lock { m_pImpl->m_mutex };
  auto it { m_pImpl->m_players.find(playerName) };
  phudAssert(m_pImpl->m_players.end() != it, "Erasing a bad player");
  m_pImpl->m_players.erase(it);
}

void PlayerCache::addIfMissing(StringView playerName) {
  const LockGuard lock { m_pImpl->m_mutex };
  m_pImpl->m_players.emplace(std::make_pair(playerName, mkUptr<Player>(Player::Params{ .name = playerName, .site = m_pImpl->m_siteName })));
}

bool PlayerCache::isEmpty() {
  const LockGuard lock { m_pImpl->m_mutex };
  return m_pImpl->m_players.empty();
}

Vector<uptr<Player>> PlayerCache::extractPlayers() {
  const LockGuard lock { m_pImpl->m_mutex };
  Vector<uptr<Player>> ret;
  ret.reserve(m_pImpl->m_players.size());
  pa::forEach(m_pImpl->m_players, [&](auto & nameToPlayer) { ret.push_back(std::move(nameToPlayer.second)); });
  m_pImpl->m_players.clear();
  return ret;
}