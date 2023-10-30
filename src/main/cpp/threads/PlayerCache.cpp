#include "entities/Player.hpp"
#include "language/assert.hpp"
#include "threads/PlayerCache.hpp"

#include <algorithm> // std::ranges::for_each
#include <map>
#include <mutex> // std::lock_guard
#include <vector>

struct [[nodiscard]] PlayerCache::Implementation final {
  std::map<std::string, std::unique_ptr<Player>, std::less<>> m_players {};
  std::mutex m_mutex {};
  std::string m_siteName;

  Implementation(std::string_view siteName): m_siteName { siteName } {}
};

PlayerCache::PlayerCache(std::string_view siteName) noexcept : m_pImpl { std::make_unique<Implementation>(siteName) } {}

PlayerCache::~PlayerCache() = default;

void PlayerCache::setIsHero(std::string_view playerName) {
  const std::lock_guard<std::mutex> lock { m_pImpl->m_mutex };
  auto it { m_pImpl->m_players.find(playerName) };
  phudAssert(m_pImpl->m_players.end() != it, "Setting hero on a bad player");
  it->second->setIsHero(true);
}

void PlayerCache::erase(std::string_view playerName) {
  const std::lock_guard<std::mutex> lock { m_pImpl->m_mutex };
  auto it { m_pImpl->m_players.find(playerName) };
  phudAssert(m_pImpl->m_players.end() != it, "Erasing a bad player");
  m_pImpl->m_players.erase(it);
}

void PlayerCache::addIfMissing(std::string_view playerName) {
  const std::lock_guard<std::mutex> lock { m_pImpl->m_mutex };
  m_pImpl->m_players.emplace(std::make_pair(playerName, std::make_unique<Player>(Player::Params{ .name = playerName, .site = m_pImpl->m_siteName })));
}

bool PlayerCache::isEmpty() {
  const std::lock_guard<std::mutex> lock { m_pImpl->m_mutex };
  return m_pImpl->m_players.empty();
}

std::vector<std::unique_ptr<Player>> PlayerCache::extractPlayers() {
  const std::lock_guard<std::mutex> lock { m_pImpl->m_mutex };
  std::vector<std::unique_ptr<Player>> ret;
  ret.reserve(m_pImpl->m_players.size());
  std::ranges::for_each(m_pImpl->m_players, [&](auto & nameToPlayer) { ret.push_back(std::move(nameToPlayer.second)); });
  m_pImpl->m_players.clear();
  return ret;
}