#pragma once

#include <filesystem> // std::filesystem::path
#include <functional> // std::function
#include <memory> // std::unique_ptr

// forward declarations
class Site;

class [[nodiscard]] PokerSiteHistory {
private:

public:
  [[nodiscard]] static std::unique_ptr<PokerSiteHistory> newInstance(const std::filesystem::path&
      historyDir);
  static std::unique_ptr<PokerSiteHistory> newInstance(auto) = delete;
  virtual ~PokerSiteHistory();

  /**
   * @returns a Site containing all the games which history files are located in
   * the given <historyDir>/history directory.
   */
  [[nodiscard]] virtual std::unique_ptr<Site> load(const std::filesystem::path& historyDir,
      std::function<void()> onProgress,
      std::function<void(std::size_t)> onSetNbFiles) = 0;
  std::unique_ptr<Site> load(auto, std::function<void()>, std::function<void(std::size_t)>) = delete;
  [[nodiscard]] static std::unique_ptr<Site> load(const std::filesystem::path& historyDir);
  std::unique_ptr<Site> load(auto historyDir) = delete;
  virtual void stopLoading() = 0;
  [[nodiscard]] virtual std::unique_ptr<Site> reloadFile(const std::filesystem::path&
      winamaxHistoryFile) = 0;
  std::unique_ptr<Site> reloadFile(auto) = delete;
  [[nodiscard]] static bool isValidHistory(const std::filesystem::path& historyDir);
  static bool isValidHistory(auto historyDir) = delete;
  [[nodiscard]] virtual std::string_view getTableNameFromTableWindowTitle(
    std::string_view tableWindowTitle) const
    = 0;
  [[nodiscard]] virtual std::filesystem::path getHistoryFileFromTableWindowTitle(
    const std::filesystem::path& historyDir,
    std::string_view tableWindowTitle) const = 0;
  std::filesystem::path getHistoryFileFromTableWindowTitle(auto historyDir,
      std::string_view tableWindowTitle) const = delete;
}; // class PokerSiteHistory
