#pragma once

#include <memory> // std::unique_ptr
#include <string_view>

// forward declarations
class TableService;
class HistoryService;
class PlayerStatistics;

/**
 * The Graphical User Interface of the Poker Head Up Display program.
 */
class [[nodiscard]] Gui final {
 public:
  struct Implementation; // public to minimise the Gui API

 private:
  std::unique_ptr<Implementation> m_pImpl;

 public:
  explicit Gui(TableService& tableService, HistoryService& historyService);
  Gui(const Gui&) = delete;
  Gui(Gui&&) = delete;
  Gui& operator=(const Gui&) = delete;
  Gui& operator=(Gui&&) = delete;
  ~Gui();
  /**
   * Starts the GUI.
   * @returns 0 if OK
   */
  [[nodiscard]] int run();
}; // class Gui
