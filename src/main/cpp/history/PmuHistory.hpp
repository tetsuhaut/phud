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
  [[nodiscard]] uptr<Site> load(const std::filesystem::path& historyDir, FunctionVoid incrementCb,
                                FunctionInt setNbFilesCb) override;
  uptr<Site> load(auto, FunctionVoid, FunctionInt) = delete;

  [[nodiscard]] static uptr<Site> load(const std::filesystem::path& historyDir);
  uptr<Site> load(auto) = delete;

  void stopLoading() override;

  [[nodiscard]] uptr<Site> reloadFile(const std::filesystem::path& winamaxHistoryFile) override;
  uptr<Site> reloadFile(auto) = delete;

  [[nodiscard]] static bool isValidHistory(const std::filesystem::path& historyDir);
  static bool isValidHistory(auto) = delete;

  [[nodiscard]] std::string_view getTableNameFromTableWindowTitle(std::string_view tableWindowTitle) const
  override;

  [[nodiscard]] std::filesystem::path getHistoryFileFromTableWindowTitle(const std::filesystem::path& historyDir,
      std::string_view tableWindowTitle) const override;
  std::filesystem::path getHistoryFileFromTableWindowTitle(auto, std::string_view) = delete;
}; // class PmuHistory