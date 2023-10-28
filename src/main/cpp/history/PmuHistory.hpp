#pragma once

#include "history/PokerSiteHistory.hpp" // Path, uptr, std::function

// forward declarations
class Site;

class [[nodiscard]] PmuHistory final : public PokerSiteHistory {

public:
  ~PmuHistory();

  /**
  * @returns a Site containing all the games which history files are located in
  * the given <historyDir>/history directory.
  */
  [[nodiscard]] uptr<Site> load(const Path& historyDir, FunctionVoid incrementCb,
                                FunctionInt setNbFilesCb) override;
  uptr<Site> load(auto, FunctionVoid, FunctionInt) = delete;

  [[nodiscard]] static uptr<Site> load(const Path& historyDir);
  uptr<Site> load(auto) = delete;

  void stopLoading() override;

  [[nodiscard]] uptr<Site> reloadFile(const Path& winamaxHistoryFile) override;
  uptr<Site> reloadFile(auto) = delete;

  [[nodiscard]] static bool isValidHistory(const Path& historyDir);
  static bool isValidHistory(auto) = delete;

  [[nodiscard]] std::string_view getTableNameFromTableWindowTitle(std::string_view tableWindowTitle) const
  override;

  [[nodiscard]] Path getHistoryFileFromTableWindowTitle(const Path& historyDir,
      std::string_view tableWindowTitle) const override;
  Path getHistoryFileFromTableWindowTitle(auto, std::string_view) = delete;
}; // class PmuHistory