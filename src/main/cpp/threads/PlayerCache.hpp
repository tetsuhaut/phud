#pragma once

#include "containers/Vector.hpp"
#include "strings/StringView.hpp"
#include "system/memory.hpp" // uptr

class Player;

class [[nodiscard]] PlayerCache final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  PlayerCache(StringView siteName) noexcept;
  PlayerCache(const PlayerCache&) = delete;
  PlayerCache(PlayerCache&&) = delete;
  PlayerCache& operator=(const PlayerCache&) = delete;
  PlayerCache& operator=(PlayerCache&&) = delete;
  ~PlayerCache();
  void setIsHero(StringView playerName);
  void erase(StringView playerName);
  void addIfMissing(StringView playerName);
  [[nodiscard]] Vector<uptr<Player>> extractPlayers();
  [[nodiscard]] bool isEmpty();
}; // class PlayerCache
