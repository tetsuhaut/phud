#pragma once

#include "mainLib/AppInterface.hpp" // Path, uptr, std::function, TableStatistics
#include "system/memory.hpp" // uptr

/**
 * The phud application.
 */
class [[nodiscard]] App final : public AppInterface {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

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
  void importHistory(const Path& historyDir,
                     FunctionVoid incrementCb = nullptr,
                     FunctionInt setNbFilesCb = nullptr,
                     FunctionVoid doneCb = nullptr) override;
  // force users to user Path
  void importHistory(auto, FunctionVoid, FunctionInt, FunctionVoid) = delete;
  void setHistoryDir(const Path& historyDir) override;
  void stopImportingHistory() override;
  [[nodiscard]] int showGui() override;

  /**
   * @return true if it has started
   */
  std::string startProducingStats(std::string_view table, std::function<void(TableStatistics&& ts)> observer) override;

  void stopProducingStats() override;
}; // class App
