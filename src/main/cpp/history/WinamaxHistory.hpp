#pragma once

#include "history/PokerSiteHistory.hpp" // std::filesystem::path, std::unique_ptr, std::function

#include <functional>

// forward declarations
class Site;

class [[nodiscard]] WinamaxHistory final : public PokerSiteHistory {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;
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
  [[nodiscard]] std::unique_ptr<Site> load(const std::filesystem::path& historyDir,
      std::function<void()> onProgress,
      std::function<void(std::size_t)> onSetNbFiles) override;
  std::unique_ptr<Site> load(auto, std::function<void()>, std::function<void(std::size_t)>) = delete;

  [[nodiscard]] static std::unique_ptr<Site> load(const std::filesystem::path& historyDir);
  std::unique_ptr<Site> load(auto) = delete;

  void stopLoading() override;

  [[nodiscard]] std::unique_ptr<Site> reloadFile(const std::filesystem::path& winamaxHistoryFile)
  override;
  std::unique_ptr<Site> reloadFile(auto) = delete;

  [[nodiscard]] static bool isValidHistory(const std::filesystem::path& historyDir);
  static bool isValidHistory(auto) = delete;

  [[nodiscard]] std::string_view getTableNameFromTableWindowTitle(std::string_view tableWindowTitle)
  const
  override;

  [[nodiscard]] std::filesystem::path getHistoryFileFromTableWindowTitle(
    const std::filesystem::path& historyDir,
    std::string_view tableWindowTitle) const override;
  std::filesystem::path getHistoryFileFromTableWindowTitle(auto, std::string_view) const = delete;
}; // class WinamaxHistory
