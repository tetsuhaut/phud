#pragma once

#include "gui/Rectangle.hpp"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#endif // __clang__

#include <windows.h> // HWND

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif // __clang__

#include <optional>
#include <string>
#include <string_view>

namespace mswindows {
/**
 * @return the list the currently visible top level windows titles
 */
[[nodiscard]] std::vector<std::string> getWindowTitles();

[[nodiscard]] std::optional<phud::Rectangle>
getTableWindowRectangle(std::string_view tableWindowTitle);

void setWindowOnTopMost(HWND above) noexcept;
} // namespace mswindows