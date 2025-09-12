#pragma once

#include "gui/TableService.hpp"
#include "gui/Rectangle.hpp"
#include "language/ErrOrRes.hpp"

#include <utility> // std::pair
#include <string>
#include <windows.h>

using ErrorOrRectangleAndName = ErrOrRes<std::pair<phud::Rectangle, std::string>>;

/**
 * Gets the last error message from the OS.
 * @return Formatted error message
 */
[[nodiscard]] std::string getLastErrorMessageFromOS();

/**
 * Gets the executable name of a window process.
 * @param window Window handle
 * @return Executable name or empty string on error
 */
[[nodiscard]] std::string getExecutableName(const HWND window);

/**
 * Converts a RECT object from the Windows API into a phud::Rectangle object.
 * @param r Windows RECT structure
 * @return phud::Rectangle equivalent
 */
[[nodiscard]] constexpr phud::Rectangle toRectangle(const RECT& r) noexcept;

/**
 * Gets the current window absolute position,
 * ensures the process owning the window is the poker app,
 * gets the name of the window this position belongs to.
 * @param tableService Service for table operations and validation
 * @param x Screen x coordinate
 * @param y Screen y coordinate
 * @return Result containing window rectangle and name, or error message
 */
[[nodiscard]] ErrorOrRectangleAndName getWindowRectangleAndName(TableService& tableService, int x, int y);

/**
 * @return the list the currently visible top level windows titles
 */
[[nodiscard]] std::vector<std::string> getWindowTitles();