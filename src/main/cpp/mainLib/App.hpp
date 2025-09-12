#pragma once

#include "mainLib/AppInterface.hpp" // std::filesystem::path, std::function, TableStatistics

#include <memory> // std::unique_ptr

// forward declarations
class Database;
class TableService;
class HistoryService;

/**
 * The phud application.
 */
class [[nodiscard]] App final : public AppInterface {
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

  /**
   * Loads the hand history into database.
   * @throws
   */
  void importHistory(const std::filesystem::path& historyDir,
                     std::function<void()> onProgress = nullptr,
                     std::function<void(std::size_t)> onSetNbFiles = nullptr,
                     std::function<void()> onDone = nullptr) override;
  // force users to user std::filesystem::path
  void importHistory(auto, std::function<void()>, std::function<void(std::size_t)>,
                     std::function<void()>) = delete;
  void setHistoryDir(const std::filesystem::path& historyDir) override;
  void stopImportingHistory() override;
  [[nodiscard]] int showGui() override;

  /**
   * @return true if it has started
   */
  std::string startProducingStats(std::string_view table,
                                  std::function < void(TableStatistics&& ts) > observer) override;

  void stopProducingStats() override;
}; // class App
