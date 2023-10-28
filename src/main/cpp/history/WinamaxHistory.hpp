#pragma once

#include "history/PokerSiteHistory.hpp" // Path, uptr, std::function

// forward declarations
class Site;

class [[nodiscard]] WinamaxHistory final : public PokerSiteHistory {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;
public:
  WinamaxHistory() noexcept;
  WinamaxHistory(const WinamaxHistory&) = delete;
  WinamaxHistory(WinamaxHistory&&) = delete;
  WinamaxHistory& operator=(const WinamaxHistory&) = delete;
  WinamaxHistory& operator=(WinamaxHistory&&) = delete;
  ~WinamaxHistory();
  /**
   * @returns a Site containing all the games which history files are located in
   * the given <historyDir>/history directory.
   */
  [[nodiscard]] uptr<Site> load(const phud::filesystem::Path& historyDir, FunctionVoid incrementCb,
                                FunctionInt setNbFilesCb) override;
  uptr<Site> load(auto, FunctionVoid, FunctionInt) = delete;

  [[nodiscard]] static uptr<Site> load(const phud::filesystem::Path& historyDir);
  uptr<Site> load(auto) = delete;

  void stopLoading() override;

  [[nodiscard]] uptr<Site> reloadFile(const phud::filesystem::Path& winamaxHistoryFile) override;
  uptr<Site> reloadFile(auto) = delete;

  [[nodiscard]] static bool isValidHistory(const phud::filesystem::Path& historyDir);
  static bool isValidHistory(auto) = delete;

  [[nodiscard]] std::string_view getTableNameFromTableWindowTitle(std::string_view tableWindowTitle) const
  override;

  [[nodiscard]] phud::filesystem::Path getHistoryFileFromTableWindowTitle(const phud::filesystem::Path& historyDir,
      std::string_view tableWindowTitle) const override;
  phud::filesystem::Path getHistoryFileFromTableWindowTitle(auto, std::string_view) const = delete;
}; // class WinamaxHistory
