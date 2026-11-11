#pragma once

#include "gui/DragAndDropWindow.hpp"
#include "gui/HudLayout.hpp" // std::string, std::string_view, std::vector

#include <memory>  // std::unique_ptr
#include <utility> // std::pair

// forward declarations
class Fl_Box;
class PlayerStatistics;

/**
 * Window displaying the stats of a player at the current table.
 */
class [[nodiscard]] PlayerIndicator final : public DragAndDropWindow {
private:
  HudLayout m_layout; // how elements are positionned on the PlayerIndicator window
  std::vector<Fl_Box*> m_boxes {}; // one for each item of m_layout, owned by this window
  std::string m_playerName;
  std::unique_ptr<PlayerStatistics> m_pStats {nullptr};
  int m_userOffsetX = 0;              // Offset X when the user does drag&drop
  int m_userOffsetY = 0;              // Offset Y when the user does drag&drop
  std::pair<int, int> m_basePosition; // Position if the user does not drag&drop

  void buildBoxes();
  void displayStats() const;

public:
  PlayerIndicator(const std::pair<int, int>& position, std::string_view playerName,
                  const HudLayout& layout);
  PlayerIndicator(const PlayerIndicator&) = delete;
  PlayerIndicator(PlayerIndicator&&) = delete;
  PlayerIndicator& operator=(const PlayerIndicator&) = delete;
  PlayerIndicator& operator=(PlayerIndicator&&) = delete;
  ~PlayerIndicator() override;

  /**
   * Displays the given stats, which can be those of another player than the previous ones.
   */
  void setStats(std::unique_ptr<PlayerStatistics> pStats);

  /**
   * Changes what is displayed, without waiting for the next stats.
   */
  void setLayout(const HudLayout& layout);

  [[nodiscard]] std::string getPlayerName() const;

  void updateBasePosition(const std::pair<int, int>& basePosition);
}; // class PlayerIndicator
