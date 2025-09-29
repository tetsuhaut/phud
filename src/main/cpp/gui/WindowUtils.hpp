#pragma once

#include "gui/Rectangle.hpp"
#include <windows.h> // HWND
#include <optional>
#include <string>
#include <string_view>

/**
 * @return the list the currently visible top level windows titles
 */
[[nodiscard]] std::vector<std::string> getWindowTitles();

[[nodiscard]] std::optional<phud::Rectangle> getTableWindowRectangle(std::string_view tableWindowTitle);

void setWindowOnTopMost(HWND above) noexcept;
