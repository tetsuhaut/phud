#pragma once

#include "filesystem/Filesystem.hpp"
#include "language/callbacks.hpp" // std::function
#include "strings/StringUtils.hpp"
#include <array>

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
  virtual void importHistory(const phud::filesystem::Path& historyDir,
                             FunctionVoid incrementCb = nullptr,
                             FunctionInt setNbFilesCb = nullptr,
                             FunctionVoid doneCb = nullptr) = 0;
  // disable implicit conversions to Path
  void importHistory(auto, FunctionVoid, FunctionInt, FunctionVoid) = delete;
  virtual void setHistoryDir(const phud::filesystem::Path& dir) = 0;

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
  [[nodiscard]] static bool isValidHistory(const phud::filesystem::Path& historyDir);
  static bool isValidHistory(auto historyDir) = delete;
}; // class AppInterface