#pragma once

#include "filesystem/Filesystem.hpp" // Path
#include "language/callbacks.hpp"
#include "system/memory.hpp" // uptr

// forward declarations
class Site;

class [[nodiscard]] PokerSiteHistory {
private:

public:
  [[nodiscard]] static uptr<PokerSiteHistory> newInstance(const phud::filesystem::Path& historyDir);
  static uptr<PokerSiteHistory> newInstance(auto) = delete;
  virtual ~PokerSiteHistory();

  /**
   * @returns a Site containing all the games which history files are located in
   * the given <historyDir>/history directory.
   */
  [[nodiscard]] virtual uptr<Site> load(const phud::filesystem::Path& historyDir, FunctionVoid incrementCb,
                                        FunctionInt setNbFilesCb) = 0;
  uptr<Site> load(auto, FunctionVoid, FunctionInt) = delete;
  [[nodiscard]] static uptr<Site> load(const phud::filesystem::Path& historyDir);
  uptr<Site> load(auto historyDir) = delete;
  virtual void stopLoading() = 0;
  [[nodiscard]] virtual uptr<Site> reloadFile(const phud::filesystem::Path& winamaxHistoryFile) = 0;
  uptr<Site> reloadFile(auto) = delete;
  [[nodiscard]] static bool isValidHistory(const phud::filesystem::Path& historyDir);
  static bool isValidHistory(auto historyDir) = delete;
  [[nodiscard]] virtual std::string_view getTableNameFromTableWindowTitle(std::string_view tableWindowTitle) const
    = 0;
  [[nodiscard]] virtual phud::filesystem::Path getHistoryFileFromTableWindowTitle(const phud::filesystem::Path& historyDir,
      std::string_view tableWindowTitle) const = 0;
  phud::filesystem::Path getHistoryFileFromTableWindowTitle(auto historyDir,
                                          std::string_view tableWindowTitle) const = delete;
}; // class PokerSiteHistory
