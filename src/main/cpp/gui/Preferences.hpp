#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

/**
 * @brief The application preferences
 */
class [[nodiscard]] Preferences final {
private:
  struct Implementation;
  std::unique_ptr<Implementation> m_pImpl;
public:
  Preferences();
  Preferences(const Preferences&) = delete;
  Preferences(Preferences&&) = delete;
  Preferences& operator=(const Preferences&) = delete;
  Preferences& operator=(Preferences&&) = delete;
  ~Preferences();

  // History directory preferences
  [[nodiscard]] std::filesystem::path getPreferredHistoDir() const;
  [[nodiscard]] std::string getHistoryDirectoryDisplayLabel() const;
  void saveHistoryDirectory(const std::filesystem::path& dir);

  // Window position and size preferences
  [[nodiscard]] std::pair<int, int> getMainWindowPosition() const;
  void saveWindowPosition(int x, int y);
  void saveWindowSize(int width, int height);

  // Generic preference operations
  void saveStringPreference(std::string_view key, std::string_view value);
  void saveIntPreference(std::string_view key, int value);
  
  [[nodiscard]] std::string getStringPreference(std::string_view key, std::string_view defaultValue = "") const;
  [[nodiscard]] int getIntPreference(std::string_view key, int defaultValue = 0) const;
}; // class Preferences