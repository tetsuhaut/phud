#pragma once

/*
 * A player indicator window is a grid of cells, in which the user places what is displayed.
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

namespace PlayerIndicatorWindow::surface {
  constexpr auto cellWidth {22};
  constexpr auto cellHeight {14};
  constexpr auto nbColumns {10};
  constexpr auto nbRows {6};
  constexpr auto fontSize {12};
} // namespace PlayerIndicatorWindow::surface
