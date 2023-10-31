#pragma once

#include <memory> // std::unique_ptr
#include <string_view>
#include <vector>

class Player;

class [[nodiscard]] PlayerCache final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  PlayerCache(std::string_view siteName) noexcept;
  PlayerCache(const PlayerCache&) = delete;
  PlayerCache(PlayerCache&&) = delete;
  PlayerCache& operator=(const PlayerCache&) = delete;
  PlayerCache& operator=(PlayerCache&&) = delete;
  ~PlayerCache();
  void setIsHero(std::string_view playerName);
  void erase(std::string_view playerName);
  void addIfMissing(std::string_view playerName);
  [[nodiscard]] std::vector<std::unique_ptr<Player>> extractPlayers();
  [[nodiscard]] bool isEmpty();
}; // class PlayerCache
