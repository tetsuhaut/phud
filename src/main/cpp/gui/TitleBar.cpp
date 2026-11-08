#include "gui/TitleBar.hpp"

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027)
#endif // _MSC_VER

#include <FL/Fl.H> // Fl::get_color
#include <FL/Fl_Window.H>
#include <FL/platform.H> // fl_xid, and windows.h on Windows

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#endif // _MSC_VER

#if defined(_WIN32)
#  include <dwmapi.h> // DwmSetWindowAttribute
#  include <tuple>    // std::ignore
#endif // _WIN32

void matchTitleBarToBackground([[maybe_unused]] const Fl_Window& window) {
#if defined(_WIN32)
  unsigned char red {0};
  unsigned char green {0};
  unsigned char blue {0};
  Fl::get_color(window.color(), red, green, blue);
  const COLORREF color = RGB(red, green, blue);
  // the result is ignored: an error only means that Windows is too old to change this color
  std::ignore = DwmSetWindowAttribute(fl_xid(&window), DWMWA_CAPTION_COLOR, &color, sizeof(color));
#endif // _WIN32
}
