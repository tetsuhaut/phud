#pragma once

#include <array>
#include <filesystem>
#include <functional> // std::function
#include <string>
#include <string_view>

struct TableStatistics;

/**
* Gui and guiDryRun can use a different App implementation by using AppInterface
*/
class [[nodiscard]] AppInterface {
public:
  virtual ~AppInterface();

  /**
   * Loads the hand history into database.
   * @throws
   */
  virtual void importHistory(const std::filesystem::path& historyDir,
                             std::function<void()> incrementCb = nullptr,
                             std::function<void(std::size_t)> setNbFilesCb = nullptr,
                             std::function<void()> doneCb = nullptr) = 0;
  // disable implicit conversions to std::filesystem::path
  void importHistory(auto, std::function<void()>, std::function<void(std::size_t)>, std::function<void()>) = delete;
  virtual void setHistoryDir(const std::filesystem::path& dir) = 0;

  virtual void stopImportingHistory() = 0;

  virtual std::string startProducingStats(std::string_view table, std::function<void(TableStatistics&& ts)> observer) = 0;
  virtual void stopProducingStats() = 0;

  /**
   * displays the GUI.
   * @return 0 when all the windows are closed, should never return other values.
  */
  [[nodiscard]] virtual int showGui() = 0;


  /**
   * @return true if executableName is the wanted Poker application.
   */
  [[nodiscard]] static bool isPokerApp(std::string_view executableName);

  /**
   * @return true if historyDir is the wanted Poker application history directory.
   */
  [[nodiscard]] static bool isValidHistory(const std::filesystem::path& historyDir);
  static bool isValidHistory(auto historyDir) = delete;
}; // class AppInterface