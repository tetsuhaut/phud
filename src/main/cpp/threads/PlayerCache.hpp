#pragma once

#include "strings/StringUtils.hpp"
#include "system/memory.hpp" // uptr

class Player;

class [[nodiscard]] PlayerCache final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

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
  [[nodiscard]] std::vector<uptr<Player>> extractPlayers();
  [[nodiscard]] bool isEmpty();
}; // class PlayerCache
