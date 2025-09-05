#pragma once

#include "gui/Rectangle.hpp" // phud::Rectangle
#include "mainLib/ProgramInfos.hpp" // ProgramInfos::*, std::string_view

/*
 * Position of the graphical elements on the main screen
 */
// +---------------------------------------------------+--> X axis
// |                 Main window title                 |
// | ------------------------------------------------- |
// | |                  menu bar                     | |
// | ------------------------------------------------- |
// | ------------------- ----------------------------  |
// | |chose history dir| | histo file name           | |
// | ------------------- ----------------------------- |
// | ------------------------------------------------- |
// | |              progress bar                     | |
// | ------------------------------------------------- |
// | ------------- ---                      ---------- |
// | |Chose table| |X|                      |Stop HUD| |
// | ------------- ---                      ---------- |
// |---------------------------------------------------|
// |                information bar                    |
// +---------------------------------------------------+
// |
// V
// Y axis

namespace MainWindow::Surface {
constexpr int elemHeight { 30 };
constexpr int buttonWidth { 170 };
constexpr int mainWindowDefaultX{ 10 };
constexpr int mainWindowDefaultY{ elemHeight };
constexpr int space { 5 };
constexpr int tableChooserWidth { 50 };
constexpr int tableChooserHeight { 30 };
constexpr int mainWindowWidth = 600;
constexpr phud::Rectangle menuBar { .x = 0, .y = 0, .w = mainWindowWidth, .h = elemHeight };
constexpr phud::Rectangle chooseHistoDirBtn { .x = space,
                                                .y = menuBar.y + menuBar.h + space,
                                                .w = buttonWidth,
                                                .h = elemHeight };
constexpr phud::Rectangle histoDirTextField { .x = chooseHistoDirBtn.w + 2 * space,
                                                .y = chooseHistoDirBtn.y,
                                                .w = mainWindowWidth - chooseHistoDirBtn.w - 3 * space,
                                                .h = elemHeight };
constexpr phud::Rectangle progressBar { .x = space,
                                          .y = chooseHistoDirBtn.y + chooseHistoDirBtn.h + space,
                                          .w = mainWindowWidth - 2 * space,
                                          .h = elemHeight };
constexpr phud::Rectangle chooseTableBtn { .x = space,
                                             .y = progressBar.y + progressBar.h + space,
                                             .w = buttonWidth,
                                             .h = elemHeight };
constexpr phud::Rectangle stopHudBtn { .x = mainWindowWidth - buttonWidth - 2 * space,
                                         .y = chooseTableBtn.y,
                                         .w = buttonWidth,
                                         .h = elemHeight };
constexpr phud::Rectangle infoBar { .x = 0,
                                      .y = chooseTableBtn.y + chooseTableBtn.h + space,
                                      .w = mainWindowWidth,
                                      .h = elemHeight };
constexpr phud::Rectangle mainWindow { .x = 0,
                                         .y = 0,
                                         .w = mainWindowWidth,
                                         .h = menuBar.h + space + chooseHistoDirBtn.h + space + progressBar.h + space + chooseTableBtn.h + space + infoBar.h };

[[nodiscard]] constexpr phud::Rectangle getTableChooserRectangle(int mainWindowX,
    int mainWindowY,
    int chooseTableBtnX) noexcept {
  const auto tcX { mainWindowX + 2 * space + chooseTableBtnX + chooseTableBtn.w };
  const auto tcY { mainWindowY + 3 * space + 2 * elemHeight };
  return { .x = tcX, .y = tcY, .w = tableChooserWidth, .h = tableChooserHeight };
}
} // namespace MainWindow::Surface

