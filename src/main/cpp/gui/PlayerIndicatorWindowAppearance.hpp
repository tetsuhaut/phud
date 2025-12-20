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
  constexpr auto width {100};
  constexpr auto statsHeight {12};
  constexpr auto playerNameHeight {12};
  constexpr auto height {1 + statsHeight + 1 + playerNameHeight + 1};
  // constexpr int
} // namespace PlayerIndicatorWindow::surface
