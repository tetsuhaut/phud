#pragma once

#include "gui/HudLayout.hpp" // CellPosition, HudItem
#include "gui/Rectangle.hpp" // phud::Rectangle

/*
 * A player indicator window is a grid of cells, in which the user places what is displayed.
 * Its look is in PlayerIndicatorWindowStyle.hpp, apart as it depends on FLTK.
 * Default layout (see HudLayout):
 */
// +------------------------> X
// | ---------------------|
// | Nb hands|VPIP|PFR|AF |
// |----------------------|
// |     player name      |
// +----------------------+
// |
// V
// Y

namespace PlayerIndicatorWindow {
  namespace surface {
    constexpr auto cellWidth {22};
    constexpr auto cellHeight {14};
    constexpr auto nbColumns {10};
    constexpr auto nbRows {6};
    constexpr auto fontSize {12};
  } // namespace surface

  /**
   * @returns where an element is displayed, relative to the top left corner of the indicator.
   * @param width in cells
   * @param zoom to display the indicator bigger than on the poker tables
   */
  [[nodiscard]] constexpr phud::Rectangle toRectangle(CellPosition position, int width,
                                                      int zoom = 1) noexcept {
    return {.x = position.column * surface::cellWidth * zoom,
            .y = position.row * surface::cellHeight * zoom,
            .w = width * surface::cellWidth * zoom,
            .h = surface::cellHeight * zoom};
  }

  [[nodiscard]] constexpr phud::Rectangle toRectangle(const HudItem& item, int zoom = 1) noexcept {
    return toRectangle(item.position, item.width, zoom);
  }
} // namespace PlayerIndicatorWindow
