#pragma once

// forward declaration
class Fl_Window;

/**
 * Gives the title bar the background color of the given window. FLTK can't do it, as the title bar
 * is drawn by the operating system: it works on Windows 11 or later, and does nothing elsewhere.
 * Must be called after each show(), as hiding a window destroys its title bar.
 */
void matchTitleBarToBackground(const Fl_Window& window);
