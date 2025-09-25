#pragma once

#include <memory> // std::unique_ptr
#include <string_view>
#include <vector>

// forward declarations
class Player;

class [[nodiscard]] PlayerCache final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  explicit PlayerCache(std::string_view siteName) noexcept;
  PlayerCache(const PlayerCache&) = delete;
  PlayerCache(PlayerCache&&) = delete;
  PlayerCache& operator=(const PlayerCache&) = delete;
  PlayerCache& operator=(PlayerCache&&) = delete;
  ~PlayerCache();
  void setIsHero(std::string_view playerName) const;
  void erase(std::string_view playerName) const;
  void addIfMissing(std::string_view playerName) const;
  [[nodiscard]] std::vector<std::unique_ptr<Player>> extractPlayers();
  [[nodiscard]] bool isEmpty() const;
}; // class PlayerCache
