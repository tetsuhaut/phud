#pragma once

#include "system/memory.hpp" // uptr
#include "strings/StringView.hpp"

// forward declaration
class AppInterface;
class PlayerStatistics;

/**
 * The Graphical User Interface of the Poker Head Up Display program.
 */
class [[nodiscard]] Gui final {
public:
  struct Implementation; // public to minimise the Gui API

private:
  uptr<Implementation> m_pImpl;

public:
  explicit Gui(AppInterface& app);
  Gui(const Gui&) = delete;
  Gui(Gui&&) = delete;
  Gui& operator=(const Gui&) = delete;
  Gui& operator=(Gui&&) = delete;
  ~Gui();
  void informUser(StringView msg);
  /**
  * Starts the GUI.
  * @returns 0 if OK
  */
  [[nodiscard]] int run();
}; // class Gui