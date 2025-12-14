#include "entities/Player.hpp"
#include "language/Validator.hpp"
#include "threads/PlayerCache.hpp"

#include <algorithm> // std::ranges::for_each
#include <map>
#include <mutex> // std::scoped_lock
#include <vector>

struct [[nodiscard]] PlayerCache::Implementation final {
  std::map<std::string, std::unique_ptr<Player>, std::less<>> m_players {};
  std::mutex m_mutex {};
  std::string m_siteName;

  explicit Implementation(const std::string_view siteName)
    : m_siteName {siteName} {}
};

PlayerCache::PlayerCache(std::string_view siteName) noexcept
  : m_pImpl {std::make_unique<Implementation>(siteName)} {}

PlayerCache::~PlayerCache() = default;

void PlayerCache::setIsHero(std::string_view playerName) const {
  const std::scoped_lock lock(m_pImpl->m_mutex);
  const auto it = m_pImpl->m_players.find(playerName);
  validation::require(m_pImpl->m_players.end() != it, "Setting hero on a bad player");
  it->second->setIsHero(true);
}

void PlayerCache::erase(std::string_view playerName) const {
  const std::scoped_lock lock(m_pImpl->m_mutex);
  const auto it = m_pImpl->m_players.find(playerName);
  validation::require(m_pImpl->m_players.end() != it, "Erasing a bad player");
  m_pImpl->m_players.erase(it);
}

void PlayerCache::addIfMissing(std::string_view playerName) const {
  const std::scoped_lock lock(m_pImpl->m_mutex);
  m_pImpl->m_players.emplace(playerName,
                             std::make_unique<Player>(Player::Params {
                                 .name = playerName, .site = m_pImpl->m_siteName, .comments = ""}));
}

bool PlayerCache::isEmpty() const {
  const std::scoped_lock lock(m_pImpl->m_mutex);
  return m_pImpl->m_players.empty();
}

std::vector<std::unique_ptr<Player>> PlayerCache::extractPlayers() {
  const std::scoped_lock lock(m_pImpl->m_mutex);
  std::vector<std::unique_ptr<Player>> ret;
  ret.reserve(m_pImpl->m_players.size());
  std::ranges::for_each(m_pImpl->m_players,
                        [&](auto& nameToPlayer) { ret.push_back(std::move(nameToPlayer.second)); });
  m_pImpl->m_players.clear();
  return ret;
}
