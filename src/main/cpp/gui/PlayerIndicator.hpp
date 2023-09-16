#pragma once

#include "containers/Pair.hpp"
#include "gui/DragAndDropWindow.hpp"
#include "strings/String.hpp" // String, StringView
#include "system/memory.hpp" // uptr

// forward declarations
class Fl_Box;
class PlayerStatistics;

/**
* Window displaying the stats of a player at the current table.
*/
class [[nodiscard]] PlayerIndicator final : public DragAndDropWindow {
private:
  uptr<Fl_Box> m_textStats;
  uptr<Fl_Box> m_textPlayerName;

public:
  PlayerIndicator(const Pair<int, int>& position, StringView playerName);
  PlayerIndicator(const PlayerIndicator&) = delete;
  PlayerIndicator(PlayerIndicator&&) = delete;
  PlayerIndicator& operator=(const PlayerIndicator&) = delete;
  PlayerIndicator& operator=(PlayerIndicator&&) = delete;
  ~PlayerIndicator();

  void setStats(const PlayerStatistics& ps);

  String getPlayerName() const;

  void refresh(StringView playerName);
}; // class PlayerIndicator
