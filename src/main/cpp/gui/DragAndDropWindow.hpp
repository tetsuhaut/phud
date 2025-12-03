#pragma once

#include "gui/Rectangle.hpp" // phud::Rectangle

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 26451 4458 26812 4244)
#endif // _MSC_VER

#include <FL/Enumerations.H> // Fl_Event
#include <FL/Fl.H>           // Fl::*
#include <FL/Fl_Double_Window.H>

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#endif // _MSC_VER

#include <functional> // std::function
#include <string_view>

/**
 * A drag and drop window.
 */
class [[nodiscard]] DragAndDropWindow : public Fl_Double_Window {
 private:
  int m_offsetX {0};
  int m_offsetY {0};
  std::function<void(int, int)> m_cb;

 public:
  DragAndDropWindow(const phud::Rectangle& r, std::string_view label,
                    std::function<void(int, int)> cb = nullptr)
    : Fl_Double_Window(r.x, r.y, r.w, r.h, label.data()),
      m_cb {std::move(cb)} {}
  DragAndDropWindow(const DragAndDropWindow&) = delete;
  DragAndDropWindow(DragAndDropWindow&&) = delete;
  DragAndDropWindow& operator=(const DragAndDropWindow&) = delete;
  DragAndDropWindow& operator=(DragAndDropWindow&&) = delete;
  ~DragAndDropWindow() override = default; // to be able to use std::unique_ptr<DragAndDropWindow>

 protected:
  /**
   * The user clicked on the DragAndDropWindow widget.
   * @returns 0 if the event was not used or understood
   * @returns 1 if the event was used and can be deleted
   */
  int handle(int e) override {
    switch (e) {
      case Fl_Event::FL_PUSH: {
        // save the current click position relative to the current window
        m_offsetX = Fl::event_x();
        m_offsetY = Fl::event_y();
        return 1;
      }

      case Fl_Event::FL_RELEASE: {
        if (m_cb) {
          const auto x = Fl_Double_Window::x();
          const auto y = Fl_Double_Window::y();
          m_cb(x, y);
          return 1;
        }

        return Fl_Double_Window::handle(e);
      }

      case Fl_Event::FL_DRAG: {
        // move this DragAndDropWindow
        Fl_Double_Window::position(Fl::event_x_root() - m_offsetX, Fl::event_y_root() - m_offsetY);
        Fl_Double_Window::redraw();
        return 1;
      }

      default:
        return Fl_Double_Window::handle(e);
    }
  }
}; // class DragAndDropWindow
