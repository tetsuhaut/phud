#pragma once

#include "statistics/TableStatistics.hpp"

#include <filesystem>
#include <functional>
#include <memory> // std::unique_ptr


// forward declarations
class Database;
class TableService;
class HistoryService;

/**
 * The phud application.
 */
class [[nodiscard]] App final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;

public:
  explicit App(std::string_view);
  App(const App&) = delete;
  App(App&&) = delete;
  App& operator=(const App&) = delete;
  App& operator=(App&&) = delete;
  ~App();

  [[nodiscard]] int showGui();
}; // class App
