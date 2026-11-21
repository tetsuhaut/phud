#pragma once

#include "gui/Rectangle.hpp" // phud::Rectangle
#include <concepts>          // std::invocable
#include <gsl/gsl>           // gsl::not_null
#include <string_view>
#include <utility> // std::forward

/**
 * Creates a widget. If a group is current (between its begin() and its end()), the widget is added
 * to it, and the group deletes the widget when it is deleted.
 * @param label not copied: it must be null-terminated, and live as long as the widget. Use
 * copy_label() for a text built at runtime.
 */
template <typename WIDGET>
[[nodiscard]] gsl::not_null<WIDGET*> createWidget(const phud::Rectangle& bounds,
                                                  std::string_view label = {}) {
  const auto [x, y, w, h] = bounds;
  return new WIDGET(x, y, w, h, label.data());
}

/**
 * Creates a widget, then calls setup(widget) to configure it.
 */
template <typename WIDGET, std::invocable<gsl::not_null<WIDGET*>> SETUP>
[[nodiscard]] gsl::not_null<WIDGET*> createWidget(const phud::Rectangle& bounds,
                                                  std::string_view label, SETUP&& setup) {
  const auto widget = createWidget<WIDGET>(bounds, label);
  std::forward<SETUP>(setup)(widget);
  return widget;
}

/**
 * Creates a widget without label, then calls setup(widget) to configure it.
 */
template <typename WIDGET, std::invocable<gsl::not_null<WIDGET*>> SETUP>
[[nodiscard]] gsl::not_null<WIDGET*> createWidget(const phud::Rectangle& bounds, SETUP&& setup) {
  return createWidget<WIDGET>(bounds, {}, std::forward<SETUP>(setup));
}
