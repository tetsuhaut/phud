#pragma once

/*
 * Position of the graphical elements on the player indicator window
 */
// +------------------------> X
// | ---------------------|
// | Nb hands|VPIP/PFR/AF |
// |----------------------|
// |     player name      |
// +----------------------+
// |
// V
// Y

namespace PlayerIndicatorWindow::surface {
  constexpr int width = 100;
  constexpr int statsHeight = 12;
  constexpr int playerNameHeight = 12;
  constexpr int height = 1 + statsHeight + 1 + playerNameHeight + 1;
  // constexpr int
} // namespace PlayerIndicatorWindow::surface
