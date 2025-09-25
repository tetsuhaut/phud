#pragma once

#include "gui/DragAndDropWindow.hpp"

#include <memory> // std::unique_ptr
#include <string>
#include <string_view>
#include <utility> // std::pair

// forward declarations
class Fl_Box;
class PlayerStatistics;

/**
* Window displaying the stats of a player at the current table.
*/
class [[nodiscard]] PlayerIndicator final : public DragAndDropWindow {
private:
  std::unique_ptr<Fl_Box> m_textStats;
  std::unique_ptr<Fl_Box> m_textPlayerName;

public:
  PlayerIndicator(const std::pair<int, int>& position, std::string_view playerName);
  PlayerIndicator(const PlayerIndicator&) = delete;
  PlayerIndicator(PlayerIndicator&&) = delete;
  PlayerIndicator& operator=(const PlayerIndicator&) = delete;
  PlayerIndicator& operator=(PlayerIndicator&&) = delete;
  ~PlayerIndicator();

  void setStats(const PlayerStatistics& ps) const;

  [[nodiscard]] std::string getPlayerName() const;

  void refresh(std::string_view playerName) const;
}; // class PlayerIndicator
