#pragma once

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027)
#endif // _MSC_VER

#include <FL/Enumerations.H> // Fl_Align, Fl_Color, Fl_Font

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#endif // _MSC_VER

/*
 * How a player indicator window looks, shared by the real indicators and by the sample of the HUD
 * editor. Its size is in PlayerIndicatorWindowAppearance.hpp.
 */
namespace PlayerIndicatorWindow::style {
  constexpr Fl_Font font {FL_HELVETICA};
  constexpr Fl_Color backgroundColor {FL_WHITE};
  constexpr Fl_Color textColor {FL_FOREGROUND_COLOR};
  constexpr Fl_Align textAlign {FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP};
} // namespace PlayerIndicatorWindow::style
