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
  int m_userOffsetX = 0;              // Offset X when the user does drag&drop
  int m_userOffsetY = 0;              // Offset Y when the user does drag&drop
  std::pair<int, int> m_basePosition; // Position if the user does not drag&drop

public:
  PlayerIndicator(const std::pair<int, int>& position, std::string_view playerName);
  PlayerIndicator(const PlayerIndicator&) = delete;
  PlayerIndicator(PlayerIndicator&&) = delete;
  PlayerIndicator& operator=(const PlayerIndicator&) = delete;
  PlayerIndicator& operator=(PlayerIndicator&&) = delete;
  ~PlayerIndicator() override;

  void setStats(const PlayerStatistics& ps) const;

  [[nodiscard]] std::string getPlayerName() const;

  void refresh(std::string_view playerName) const;

  void updateBasePosition(const std::pair<int, int>& basePosition);
}; // class PlayerIndicator
