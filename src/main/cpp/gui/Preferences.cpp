#include "gui/Preferences.hpp"
#include "gui/MainWindowAppearance.hpp"
#include "gui/MainWindowLabel.hpp"
#include "mainLib/ProgramInfos.hpp"
#include "filesystem/FileUtils.hpp"
#include "log/Logger.hpp"

#include <FL/Fl_Preferences.H>

#include <FL/Fl.H>

static Logger LOG { CURRENT_FILE_NAME };
static constexpr int MAX_PATH_LENGTH { 260 };

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

struct [[nodiscard]] Preferences::Implementation final {
  Fl_Preferences m_preferences;

  Implementation() 
    : m_preferences(Fl_Preferences::USER,
        ProgramInfos::APP_SHORT_NAME.data(), ProgramInfos::APP_SHORT_NAME.data()) {}
}; // struct Preferences::Implementation

Preferences::Preferences()
  : m_pImpl { std::make_unique<Implementation>() } {}

Preferences::~Preferences() = default;

std::filesystem::path Preferences::getPreferredHistoDir() const {
  const auto& dir { getStringPreference(MainWindow::Label::preferencesKeyChosenDir.data()) };
  const auto& pathDir { fs::path(dir) };
  // Return path only if it's not empty and the directory exists
  return (!pathDir.empty() and pf::isDir(pathDir)) ? pathDir : "";
}

std::string Preferences::getHistoryDirectoryDisplayLabel() const {
  const auto& historyDir = getPreferredHistoDir();
  if (historyDir.empty()) {
    return std::string(MainWindow::Label::NO_HAND_HISTORY_DIRECTORY_SELECTED);
  }
  
  const auto dirString = historyDir.string();
  // Double-check: if string conversion somehow results in empty string, return the no-directory message
  if (dirString.empty()) {
    return std::string(MainWindow::Label::NO_HAND_HISTORY_DIRECTORY_SELECTED);
  }
  
  return dirString;
}

void Preferences::saveHistoryDirectory(const std::filesystem::path& dir) {
  saveStringPreference(MainWindow::Label::preferencesKeyChosenDir, dir.string());
}

std::pair<int, int> Preferences::getMainWindowPosition() const {
  /* get the previous width and height from preferences, if any */
  int width, height;
  m_pImpl->m_preferences.get(MainWindow::Label::width.data(), width, MainWindow::Screen::mainWindow.w);
  m_pImpl->m_preferences.get(MainWindow::Label::height.data(), height, MainWindow::Screen::mainWindow.h);
  
  /* compute the center position */
  int dummyX, dummyY, screenWidth, screenHeight;
  Fl::screen_xywh(dummyX, dummyY, screenWidth, screenHeight);
  const auto initX { (screenWidth - width) / 2 };
  const auto initY { (screenHeight - height) / 2 };
  
  /* get the previous position from preferences. if none, use the center position */
  int x, y;
  m_pImpl->m_preferences.get(MainWindow::Label::x.data(), x, initX);
  m_pImpl->m_preferences.get(MainWindow::Label::y.data(), y, initY);
  
  return { x, y };
}

void Preferences::saveWindowPosition(int x, int y) {
  saveIntPreference(MainWindow::Label::x, x);
  saveIntPreference(MainWindow::Label::y, y);
}

void Preferences::saveWindowSize(int width, int height) {
  saveIntPreference(MainWindow::Label::width, width);
  saveIntPreference(MainWindow::Label::height, height);
}

void Preferences::saveStringPreference(std::string_view key, std::string_view value) {
  if (0 == m_pImpl->m_preferences.set(key.data(), value.data())) {
    LOG.error<"Couldn't save '{}' into the preferences repository.">(key);
  }
}

void Preferences::saveIntPreference(std::string_view key, int value) {
  if (0 == m_pImpl->m_preferences.set(key.data(), value)) {
    LOG.error<"Couldn't save '{}' into the preferences repository.">(key);
  }
}

std::string Preferences::getStringPreference(std::string_view key, std::string_view defaultValue) const {
  char buffer[MAX_PATH_LENGTH + 1] { '\0' };
  m_pImpl->m_preferences.get(key.data(), buffer, defaultValue.data(), MAX_PATH_LENGTH);
  buffer[MAX_PATH_LENGTH] = '\0';
  return buffer;
}

int Preferences::getIntPreference(std::string_view key, int defaultValue) const {
  int value;
  m_pImpl->m_preferences.get(key.data(), value, defaultValue);
  return value;
}