#pragma once

#include "constants/ProgramInfos.hpp" // ProgramInfos::*, std::string_view
#include "gui/Rectangle.hpp" // phud::Rectangle

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
// |                                        ---------- |
// | Watching <table name>                  |Stop HUD| |
// |                                        ---------- |
// |---------------------------------------------------|
// |                information bar                    |
// +---------------------------------------------------+
// |
// V
// Y axis

namespace MainWindow::Screen {
constexpr int elemHeight { 30 };
constexpr int buttonWidth { 170 };
constexpr int mainWindowDefaultX{ 10 };
constexpr int mainWindowDefaultY{ elemHeight };
constexpr int space { 5 };
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
constexpr phud::Rectangle watchedTableLabel { .x = space,
                                              .y = progressBar.y + progressBar.h + space,
                                              .w = mainWindowWidth - buttonWidth - 3 * space,
                                              .h = elemHeight };
constexpr phud::Rectangle stopHudBtn { .x = mainWindowWidth - buttonWidth - space,
                                         .y = progressBar.y + progressBar.h + space,
                                         .w = buttonWidth,
                                         .h = elemHeight };
constexpr phud::Rectangle infoBar { .x = 0,
                                      .y = progressBar.y + progressBar.h + space + elemHeight + space,
                                      .w = mainWindowWidth,
                                      .h = elemHeight };
constexpr phud::Rectangle mainWindow { .x = 0,
                                         .y = 0,
                                         .w = mainWindowWidth,
                                         .h = menuBar.h + space + chooseHistoDirBtn.h + space + progressBar.h + space + elemHeight + space + infoBar.h };

} // namespace MainWindow::Screen
