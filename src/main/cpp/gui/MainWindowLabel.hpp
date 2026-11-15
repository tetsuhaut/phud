#pragma once

#include "constants/ProgramInfos.hpp"

#include <string_view>

namespace MainWindow::Label {
  constexpr std::string_view x {"x"};
  constexpr std::string_view y {"y"};
  constexpr std::string_view width {"width"};
  constexpr std::string_view height {"height"};
  constexpr std::string_view chooseHistoDir {"Choose History Directory"};
  constexpr std::string_view chooseHistoDirText {"Choose a directory containing history files"};
  constexpr std::string_view stopHud {"Stop HUD"};
  constexpr std::string_view startHud {"Start HUD"};
  constexpr std::string_view hudStopped {"HUD stopped"};
  constexpr std::string_view reimportHistoryMenu {"&File/&Reimport history"};
  constexpr std::string_view noHistoryDirToReimport {"Choose a history directory first"};
  constexpr std::string_view importInProgress {"An import is already in progress"};
  constexpr std::string_view historyImported {"History imported"};
  constexpr std::string_view customizeHudMenu {"&HUD/&Customize..."};
  constexpr std::string_view invalidChoice {
      "The chosen entry is not a valid Winamax history directory"};
  constexpr std::string_view chooseHistoryDirectory {"Choose the hand history directory"};
  constexpr auto mainWindowTitle {ProgramInfos::APP_LONG_NAME_AND_VERSION};
  constexpr std::string_view turnLeft {"Rotate counterclock wise"};
  constexpr std::string_view turnRight {"Rotate clock wise"};
  constexpr std::string_view HAND_HISTORY_EXISTS {"The hand history directory exists"};
  constexpr std::string_view WELCOME {"Hello you :)"};
  constexpr std::string_view CHOOSE_HAND_HISTORY_DIRECTORY {"Choose a hand history directory"};
  constexpr std::string_view NO_HAND_HISTORY_DIRECTORY_SELECTED {
      "<no hand history directory selected>"};
  constexpr std::string_view fileChoiceError {"File choice error"};
  constexpr std::string_view noPlayerIndicators {"No player indicators to display"};
  constexpr std::string_view noPokerTableDetected {"No poker table detected"};
  constexpr std::string_view watchingTable {"Watching table: {}"};
  constexpr std::string_view watchingMultipleTables {"Watching {} poker tables"};
} // namespace MainWindow::Label
