#include "constants/TableConstants.hpp"
#include "entities/Player.hpp"
#include "entities/Seat.hpp"
#include "gui/Gui.hpp"  // std::unique_ptr, std::make_unique
#include "gui/TableService.hpp"
#include "gui/HistoryService.hpp"
#include "gui/MainWindowAppearance.hpp" // Surface::
#include "gui/MainWindowColor.hpp" // Color::
#include "gui/MainWindowLabel.hpp" // Label::
#include "gui/PlayerIndicator.hpp" // DragAndDropWindow, Fl_Double_Window
#include "gui/Position.hpp" // buildPlayerIndicatorPosition()
#include "gui/WindowUtils.hpp"
#include "gui/Preferences.hpp"
#include "gui/TableWatcher.hpp"
#include "language/assert.hpp" // phudAssert
#include "language/Either.hpp" // ErrOrRes
#include "log/Logger.hpp" // CURRENT_FILE_NAME, fmt::*, Logger, StringLiteral
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "strings/StringUtils.hpp"
#include "filesystem/FileUtils.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadPool.hpp"
#include <frozen/unordered_map.h>
#include <gsl/gsl> // gsl::finally

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027 )
#endif  // _MSC_VER

#include <FL/Enumerations.H> // Fl_Event
#include <FL/Fl.H> // Fl::awake
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Tabs.H>
#include <FL/platform.H> // fl_xid

#if defined(_MSC_VER)  // end of specific msvc warnings removal
#  pragma warning(pop)
#endif  // _MSC_VER

#include <windows.h> // RECT, GetWindowThreadProcessId, MAX_PATH, WindowFromPoint, GetWindowRect, GetWindowText, SetWindowPos
#include <psapi.h>  // GetModuleFileNameEx
#include <concepts> // requires
#include <unordered_map>

/*
* From http://www.fltk.org/doc-1.3/advanced.html#advanced_multithreading:
* - the main thread, i.e. the one running the run() function, draws widgets.
* - any other thread requesting widgets drawing must post a callback to the main thread using the
*   awake() function. It is possible to draw directly using lock()/unlock(), but the cb passing
*   method is less risky as lock()/unlock() blocks the main thread.
*/

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

static Logger LOG { CURRENT_FILE_NAME };

namespace {
  /* in anonymous namespace as type definitions can't be static */
  enum class [[nodiscard]] FileChoiceStatus : short { ok = 0, error = -1, cancel = 1 };

  namespace FltkSkin {
    /* from the int Fl::scheme(const char * s) documentation */
    constexpr std::string_view none { "none" };
    constexpr std::string_view base { "base" };
    constexpr std::string_view gleam { "gleam" };
    constexpr std::string_view gtkplus { "gtk+" };
    constexpr std::string_view plastic { "plastic" };
  } // namespace FltkSkin

  template<typename T>
  [[nodiscard]] constexpr std::vector<T> toVector(std::span<const T> span) {
    return std::vector<T>(span.begin(), span.end());
  }

} // anonymous namespace

/**
 * Defines the concept of a function taking no argument and returning void
 */
template<typename F>
concept VoidNullaryFunction = requires(F f) {
  // calling f returns void
  { f() } -> std::same_as<void>;
} and std::is_invocable_v<F> and !std::is_invocable_v<F, int>;

/**
 * Schedules a function to be executed by the main GUI thread during the next message handling cycle.
 * @param aTask the function to be executed
 */
template<VoidNullaryFunction TASK> static void scheduleUITask(TASK&& aTask) {
  using TaskType = std::decay_t<TASK>;
  Fl::awake([](void* hiddenTask) {
    auto task { std::unique_ptr<TaskType>(static_cast<TaskType*>(hiddenTask)) };
    try {
      (*task)();
    }
    catch (const std::exception& e) {
      LOG.error<"Error in UI task: {}">(e.what());
    }
    catch (...) {
      LOG.error<"Unknown error in UI task">();
    }
  }, std::make_unique<TaskType>(std::forward<TASK>(aTask)).release());
}

/**
 * An Fl_Double_Window with disabled 'close on Esc key' behavior
 */
class [[nodiscard]] MyMainWindow final : public Fl_Double_Window {
public:
  using Fl_Double_Window::Fl_Double_Window;
  MyMainWindow(const MyMainWindow&) = delete;
  MyMainWindow(MyMainWindow&&) = delete;
  MyMainWindow& operator=(const MyMainWindow&) = delete;
  MyMainWindow& operator=(MyMainWindow&&) = delete;
  ~MyMainWindow() = default;
  int handle(int e) override {
    switch (e) {
      // disable the 'close on Esc key' default behavior
      case Fl_Event::FL_SHORTCUT: return (FL_Escape == Fl::event_key()) ? 1 : Fl_Double_Window::handle(e);

      default: return Fl_Double_Window::handle(e);
    }
  }
}; // class MyMainWindow


struct [[nodiscard]] Gui::Implementation final {
  TableService& m_tableService;
  HistoryService& m_historyService;
  Fl_Group* m_winamaxGroup { nullptr };
  Fl_Group* m_pmuGroup { nullptr };
  Fl_Tabs* m_tabs { nullptr };
  MyMainWindow* m_mainWindow { nullptr };
  std::unique_ptr<Preferences> m_preferences { std::make_unique<Preferences>() };
  Fl_Button* m_chooseHistoDirBtn { nullptr };
  Fl_Button* m_stopHudBtn { nullptr };
  Fl_Box* m_histoDirTextField { nullptr };
  Fl_Progress* m_progressBar { nullptr };
  Fl_Box* m_watchedTablesLabel { nullptr };
  Fl_Box* m_infoBar { nullptr };
  Fl_Menu_Bar* m_menuBar { nullptr };
  std::unique_ptr<TableWatcher> m_tableWatcher { nullptr };
  std::unordered_map<std::string, std::array<std::unique_ptr<PlayerIndicator>, TableConstants::MAX_SEATS>> m_playerIndicators {};

  explicit Implementation(TableService& tableService, HistoryService& historyService) 
    : m_tableService { tableService }
    , m_historyService { historyService } {}

  Implementation(const Implementation&) = delete;
  Implementation(Implementation&&) = delete;
  Implementation& operator=(const Implementation&) = delete;
  Implementation& operator=(Implementation&&) = delete;

  ~Implementation() { 
    if (m_tableWatcher) {
      m_tableWatcher->stop();
    }
    ThreadPool::stop(); 
  }
}; // struct Gui::Implementation

template <typename T>
[[nodiscard]] constexpr static std::unique_ptr<T> mkWidget(const phud::Rectangle& r, std::string_view label = {}) {
  // if T can be constructed with a label and we have a non-empty label, use it
  if constexpr (requires { T(r.x, r.y, r.w, r.h, label.data()); }) {
    if (!label.empty()) { return std::make_unique<T>(r.x, r.y, r.w, r.h, label.data()); }
  }
  return std::make_unique<T>(r.x, r.y, r.w, r.h);
}

[[nodiscard]] inline std::unique_ptr<Fl_Native_File_Chooser> buildDirectoryChooser(
  const Preferences& preferences) {
  const auto& startDir { preferences.getPreferredHistoDir() };
  auto pHistoryChoser { std::make_unique<Fl_Native_File_Chooser>() };
  pHistoryChoser->title(MainWindow::Label::chooseHistoryDirectory.data());
  pHistoryChoser->type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
  pHistoryChoser->directory(startDir.string().c_str());
  return pHistoryChoser;
}

/**
* Displays a message in the info bar, when the message is known at compile time.
*/
template<const std::string_view& MSG>
static inline void informUser(Gui::Implementation& aSelf) {
  LOG.debug<__func__>();
  scheduleUITask([infoBar = aSelf.m_infoBar, msg = std::string(MSG)]() {
    infoBar->copy_label(msg.c_str());
  });
}

/**
* Displays a message in the info bar.
*/
static inline void informUser(Gui::Implementation& aSelf, std::string_view aMsg) {
  LOG.debug<__func__>();
  scheduleUITask([infoBar = aSelf.m_infoBar, msg = std::string(aMsg)]() {
    infoBar->copy_label(msg.c_str());
  });
}

static inline void setWindowOnTopMost(const Fl_Window& above) {
  /* from https://www.fltk.org/newsgroups.php?s39452+gfltk.general+v39464 */
  SetWindowPos(fl_xid(&above), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
}

static inline void updateTablePlayerIndicators(std::array<std::unique_ptr<PlayerIndicator>, TableConstants::MAX_SEATS>& playerIndicators, const phud::Rectangle& tablePosition,
  TableStatistics tableStatistics) {
  const auto heroSeat { tableStatistics.getHeroSeat() };
  const auto& seats { tableStatistics.getSeats() };

  LOG.debug<"Processing {} seats for player indicators">(seats.size());
  for (const auto& seat : seats) {
    if (auto ps { tableStatistics.extractPlayerStatistics(seat) }; nullptr != ps) {
      LOG.debug<"Creating/updating indicator for player '{}' at seat {}">(ps->getPlayerName(), static_cast<int>(seat));
      const auto& pos { buildPlayerIndicatorPosition(seat, heroSeat, tableStatistics.getMaxSeat(), tablePosition) };
      // update the PlayerIndicators with the latest stats.
      // -1 < seat < nbSeats, 1 < nbSeats < 11
      auto& playerIndicator { playerIndicators.at(tableSeat::toArrayIndex(seat)) };

      if (nullptr == playerIndicator) {
        LOG.debug<"Creating new PlayerIndicator for '{}'">(ps->getPlayerName());
        playerIndicator = std::make_unique<PlayerIndicator>(pos, ps->getPlayerName());
      }
      else if (ps->getPlayerName() != playerIndicator->getPlayerName()) {
        LOG.debug<"Refreshing PlayerIndicator for '{}'">(ps->getPlayerName());
        playerIndicator->refresh(ps->getPlayerName());
      }
      playerIndicator->setStats(*ps);
      setWindowOnTopMost(*playerIndicator);
      playerIndicator->show();
      LOG.debug<"PlayerIndicator shown for '{}'">(ps->getPlayerName());
    }
    else {
      LOG.debug<"No player statistics for seat {}, clearing indicator">(static_cast<int>(seat));
      // clear player indicators
      playerIndicators.at(tableSeat::toArrayIndex(seat)).reset();
    }
  }
}

static inline void removeUselessPlayerIndicators(
  std::unordered_map<std::string, std::array<std::unique_ptr<PlayerIndicator>, 10>>& playerIndicators,
  const std::span<const std::string> tableNames,
  TableService& tableService) {
  LOG.info<"Delete table indicators for removed table(s)">();
  for (auto it = playerIndicators.begin(); it != playerIndicators.end();) {
    if (tableNames.end() == std::find(tableNames.begin(), tableNames.end(), it->first)) {
      // Stop monitoring this table
      tableService.stopProducingStats();

      // Clear all player indicators for this table
      for (auto& pi : it->second) {
        pi.reset();
      }
      LOG.debug<"Removed player indicators for table: {}">(it->first);
      it = playerIndicators.erase(it);
    } else {
      ++it;
    }
  }
}

static inline void updateUsefulPlayerIndicators(
  std::unordered_map<std::string, std::array<std::unique_ptr<PlayerIndicator>, TableConstants::MAX_SEATS>>& playerIndicators,
  const std::span<const std::string> tableNames,
  TableService& tableService) {
  LOG.info<"Create/Update table indicators for {} table(s)">(tableNames.size());
  for (const auto& tableName : tableNames) {
    // Create entry if it doesn't exist
    if (playerIndicators.find(tableName) == playerIndicators.end()) {
      playerIndicators[tableName] = std::array<std::unique_ptr<PlayerIndicator>, TableConstants::MAX_SEATS>{};
      LOG.debug<"Created player indicators array for table: {}">(tableName);
      
      // Start monitoring this table for statistics
      auto observer = [&playerIndicators, &tableName](TableStatistics&& stats) {
        LOG.debug<"Observer called for table '{}'">(tableName);
        scheduleUITask([&playerIndicators, tableName, stats = std::move(stats)]() mutable {
          LOG.debug<"Scheduled UI task executing for table '{}'">(tableName);
          // Find window position by title
          if (const auto& hwnd {FindWindow(nullptr, tableName.c_str())}; nullptr != hwnd) {
            LOG.debug<"Found window for table '{}'">(tableName);
            if (RECT rect; 0 != GetWindowRect(hwnd, &rect)) {
              phud::Rectangle tableRect = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
              
              // Update PlayerIndicators with real statistics
              if (playerIndicators.find(tableName) != playerIndicators.end()) {
                LOG.debug<"Updating player indicators for table '{}'">(tableName);
                updateTablePlayerIndicators(playerIndicators[tableName], tableRect, std::move(stats));
              } else {
                LOG.warn<"Player indicators not found for table '{}'">(tableName);
              }
            } else {
              LOG.warn<"Could not get window rect for table '{}'">(tableName);
            }
          } else {
            LOG.warn<"Window not found for table '{}'">(tableName);
          }
        });
      };
      if (const auto& errorMsg { tableService.startProducingStats(tableName, observer) }; !errorMsg.empty()) {
        LOG.error<"Failed to start monitoring table '{}': {}">(tableName, errorMsg);
      }
    }
  }
}

static inline void managePlayerIndicatorsForTables(
  std::unordered_map<std::string, std::array<std::unique_ptr<PlayerIndicator>, 10>>& playerIndicators,
  const std::span<const std::string> tableNames,
  TableService& tableService) {
  // Remove indicators for tables no longer detected
  removeUselessPlayerIndicators(playerIndicators, tableNames, tableService);
  // Create/update indicators for current tables
  updateUsefulPlayerIndicators(playerIndicators, tableNames, tableService);
}

/**
* Called by the GUI (button component) when the user clicks on the 'exit'
* button.
*/
static inline void exitCb(Fl_Widget* const mainWindow, void* hidden) {
  LOG.debug<__func__>();
  LOG.info<"Stopping the GUI.">();
  auto& impl { *static_cast<Gui::Implementation*>(hidden) };
  
  // Stop watching for poker tables first
  if (impl.m_tableWatcher) {
    impl.m_tableWatcher->stop();
  }
  
  /* save the main window position */
  impl.m_preferences->saveWindowPosition(mainWindow->x(), mainWindow->y());
  impl.m_preferences->saveWindowSize(mainWindow->w(), mainWindow->h());

  /* close all subwindows */
  while (Fl::first_window()) { 
    Fl::first_window()->hide();
    Fl::check(); // Force FLTK to treat events
  }
}

static void onImportProgress(Fl_Progress* progressBar) {
  scheduleUITask([pb = progressBar]() {
    pb->value(pb->value() + 1);
    pb->copy_label(fmt::format("{}/{}", limits::toInt(pb->value()), limits::toInt(pb->maximum())).c_str());
  });
}

static void onSetNbFiles(Fl_Progress* progressBar, std::size_t nbFilesToLoad) {
  scheduleUITask([pb = progressBar, nbFilesToLoad]() {
    // as the progress bar boundaries are float values
    pb->maximum(static_cast<float>(nbFilesToLoad));
  });
}

static void onImportDone(Fl_Button* chooseHistoDirBtn) {
  scheduleUITask([chb = chooseHistoDirBtn]() {
    chb->activate();
  });
}

/**
 * Called by the GUI event loop when the user chosed a valid history dir.
 * Starts the import process for a valid history directory.
 * Updates UI components and starts the background import.
 */
static inline void importDirAwakeCb(Fl_Progress* progressBar, Fl_Button* chooseHistoDirBtn, HistoryService& historyService, const fs::path& dir) {
  LOG.debug<__func__>();
  
  try {
    LOG.info<"The import directory '{}' is valid">(dir.string());
    
    // UI update - already running in UI thread, no need for scheduleUITask
    progressBar->activate();
    chooseHistoDirBtn->deactivate();
    
    // Start import through business service
    LOG.info<"About to call historyService.startImport">();
    historyService.importHistory(dir,
      // update the progress bar during the import
      [progressBar]() { onImportProgress(progressBar); },
      // when we know the number of files to import, setup the progress bar
      [progressBar](std::size_t nb) { onSetNbFiles(progressBar, nb); },
      // import completion callback
      [chooseHistoDirBtn]() { onImportDone(chooseHistoDirBtn); });    
    LOG.info<"historyService.importHistory completed">();
  }
  catch (const std::exception& e) {
    LOG.error<"Exception in importDirAwakeCb: {}">(e.what());
  }
  catch (...) {
    LOG.error<"Unknown exception in importDirAwakeCb">();
  }
}

template<typename WIDGET>
[[nodiscard]] static inline gsl::not_null<WIDGET*> createWidget(
  const phud::Rectangle& bounds, std::string_view label) {
  const auto& [x, y, w, h] { bounds };
  return new WIDGET(x, y, w, h, label.data());
}

template<typename WIDGET>
[[nodiscard]] static inline gsl::not_null<WIDGET*> createWidget(
  const phud::Rectangle& bounds, std::string_view label, auto setupFunction) {
  auto widget { createWidget<WIDGET>(bounds, label) };
  setupFunction(widget);
  return widget;
}

template<typename WIDGET>
[[nodiscard]] static inline gsl::not_null<WIDGET*> createWidget(
  const phud::Rectangle& bounds, auto setupFunction) {
  const auto& [x, y, w, h] { bounds };
  auto widget = new WIDGET(x, y, w, h);
  setupFunction(widget);
  return widget;
}

[[nodiscard]] static inline gsl::not_null<Fl_Menu_Bar*> buildMenuBar(Gui::Implementation* impl) {
  return createWidget<Fl_Menu_Bar>(MainWindow::Screen::menuBar, [impl](Fl_Menu_Bar* menu) {
    menu->add("&File/E&xit", 0, exitCb, impl);
    menu->box(FL_NO_BOX);
  });
}

namespace DirectoryChoiceHandler {
void handleOk(std::string_view dirName, Gui::Implementation& self) {
  const auto& dir { fs::path { dirName } };
  LOG.info<"the user chose to import the directory '{}'">(dir.string());

  if (self.m_historyService.isValidHistory(dir)) {
    self.m_preferences->saveHistoryDirectory(dir);
    self.m_historyService.setHistoryDir(dir); // Set the history directory in the service
    
    // Update directory label immediately (synchronously) to avoid pointer issues
    const auto& label = self.m_preferences->getHistoryDirectoryDisplayLabel();
    self.m_histoDirTextField->copy_label(label.c_str());
    
    // Schedule the rest of the UI updates and import
    scheduleUITask([pb = self.m_progressBar,
                    chb = self.m_chooseHistoDirBtn,
                    historyService = &self.m_historyService,
                    dir]() { 
      try {
        LOG.info<"About to start import process">();
        // Start import
        importDirAwakeCb(pb, chb, *historyService, dir);
        LOG.info<"Import process started successfully">();
      }
      catch (const std::exception& e) {
        LOG.error<"Exception in UI task: {}">(e.what());
      }
      catch (...) {
        LOG.error<"Unknown exception in UI task">();
      }
    });
  }
  else {
    LOG.info<"the chosen directory '{}' is not a valid history dir">(dir.string());
    informUser<MainWindow::Label::invalidChoice>(self);
  }
}

void handleError(Fl_Native_File_Chooser* chooser, Gui::Implementation& self) {
  if (nullptr == chooser->errmsg()) { 
    informUser<MainWindow::Label::fileChoiceError>(self); 
  }
  else { 
    informUser(self, chooser->errmsg()); 
  }
}
} // namespace DirectoryChoiceHandler

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildChooseHistoDirBtn(
  Gui::Implementation& aSelf) {
  // Called by the GUI (button component) when the user clicks on the 'choose history directory' button.
  // Displays a directory chooser window.
  auto choseHistoDirCb = [](Fl_Widget*, void* hiddenSelf) {
    LOG.debug<__func__>();
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
    informUser<MainWindow::Label::chooseHistoDirText>(self);
    auto dirChoser { buildDirectoryChooser(*self.m_preferences) };

    switch (FileChoiceStatus(dirChoser->show())) {
    case FileChoiceStatus::ok:
      DirectoryChoiceHandler::handleOk(dirChoser->filename(), self);
      break;
    case FileChoiceStatus::error: [[unlikely]]
      DirectoryChoiceHandler::handleError(dirChoser.get(), self);
      break;
    case FileChoiceStatus::cancel: [[fallthrough]];
    default: /* nothing to do */ break;
    }
  };

  return createWidget<Fl_Button>(MainWindow::Screen::chooseHistoDirBtn, 
                                 MainWindow::Label::chooseHistoDir, 
                                 [&aSelf, choseHistoDirCb](Fl_Button* btn) {
    /* setup the button for choosing which history directory to import into database */
    btn->callback(choseHistoDirCb, &aSelf);
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildStopHudBtn(Gui::Implementation& aSelf) {
  
  // Called by the GUI (button component) when the user clicks on the 'stop HUD'
  auto stopHudCb = [](Fl_Widget* button, void* hiddenSelf) {
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
    LOG.debug<__func__>();
    // Use business service to stop monitoring
    self.m_tableService.stopProducingStats();
    // kill PlayerIndicators
    for (auto& [tableName, indicators] : self.m_playerIndicators) {
      std::ranges::for_each(indicators, [](auto& pi) { pi.reset(); });
    }
    self.m_playerIndicators.clear();
    button->deactivate();
    informUser<MainWindow::Label::noPlayerIndicators>(self);
    Fl::redraw();
  };

  return createWidget<Fl_Button>(MainWindow::Screen::stopHudBtn, 
                                 MainWindow::Label::stopHud, 
                                 [&aSelf, stopHudCb](Fl_Button* btn) {
    btn->callback(stopHudCb, &aSelf);
    btn->deactivate();
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildHistoDirTextField(const Preferences& pref) {
  return createWidget<Fl_Box>(MainWindow::Screen::histoDirTextField, [&pref](Fl_Box* box) {
    /* setup the text box to display the chosen directory */
    const auto& label = pref.getHistoryDirectoryDisplayLabel();
    box->copy_label(label.c_str());
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Progress*> buildProgressBar() {
  return createWidget<Fl_Progress>(MainWindow::Screen::progressBar, [](Fl_Progress* progress) {
    progress->color(MainWindow::Color::progressBarBackgroundColor);
    progress->selection_color(MainWindow::Color::progressBarForegroundColor);
    progress->labelcolor(MainWindow::Color::progressBarTextColor);
    progress->minimum(0);
    progress->maximum(1);
    progress->deactivate();
    //progress->box(FL_NO_BOX);
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildWatchedTablesLabel() {
  return createWidget<Fl_Box>(MainWindow::Screen::watchedTableLabel, MainWindow::Label::noPokerTableDetected, [](Fl_Box* box) {
    box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildInfoBar() {
  return createWidget<Fl_Box>(MainWindow::Screen::infoBar, MainWindow::Label::WELCOME);
}

static inline std::string getWatchedTableLabel(std::span<const std::string> tableNames) {
  if (tableNames.empty()) {
    return MainWindow::Label::noPokerTableDetected.data();
  }
  if (tableNames.size() == 1) {
    return std::format(MainWindow::Label::watchingTable, tableNames.front());
  }
  return fmt::format(MainWindow::Label::watchingMultipleTables, tableNames.size());
}

[[nodiscard]] static inline std::unique_ptr<TableWatcher> buildTableWatcher(
  Fl_Box* pWatchedTableLabel, 
  std::unordered_map<std::string, std::array<std::unique_ptr<PlayerIndicator>, TableConstants::MAX_SEATS>>& playerIndicators,
  TableService& tableService) {
  
  TableWatcher::TablesChangedCallback onTablesChangedCb {
    [pWatchedTableLabel, &playerIndicators, &tableService](std::span<const std::string> tableNames) {
      scheduleUITask([pWatchedTableLabel, &playerIndicators, &tableService, tableNames = toVector(tableNames)]() {
        // Update label
        const auto& label { getWatchedTableLabel(tableNames) };
        pWatchedTableLabel->copy_label(label.c_str());
        // Manage PlayerIndicators for all detected tables
        managePlayerIndicatorsForTables(playerIndicators, tableNames, tableService);
      });
    }
  };
  
  return std::make_unique<TableWatcher>(onTablesChangedCb);
}

static inline gsl::not_null<MyMainWindow*> buildMainWindow(Gui::Implementation* impl) {
  Fl::scheme(FltkSkin::gleam.data());  // select the look & feel
  Fl::visual(Fl_Mode::FL_DOUBLE | Fl_Mode::FL_INDEX);  // enhance look where needed
  const auto [mx, my, mw, mh] { MainWindow::Screen::mainWindow };
  auto ret { new MyMainWindow(mx, my, mw, mh, MainWindow::Label::mainWindowTitle.data()) };
  const auto [x, y] { impl->m_preferences->getMainWindowPosition() };
  ret->position(x, y);
  ret->callback(exitCb, impl);
  return ret;
}

Gui::Gui(TableService& tableService, HistoryService& historyService)
  : m_pImpl { std::make_unique<Gui::Implementation>(tableService, historyService) } {
  LOG.debug<__func__>();
  m_pImpl->m_mainWindow = buildMainWindow(m_pImpl.get());
  m_pImpl->m_menuBar = buildMenuBar(m_pImpl.get());
  m_pImpl->m_chooseHistoDirBtn = buildChooseHistoDirBtn(*m_pImpl);
  m_pImpl->m_histoDirTextField = buildHistoDirTextField(*m_pImpl->m_preferences);
  m_pImpl->m_progressBar = buildProgressBar();
  m_pImpl->m_watchedTablesLabel = buildWatchedTablesLabel();
  if (const auto dir { m_pImpl->m_preferences->getPreferredHistoDir() }; pf::isDir(dir)) {
    m_pImpl->m_historyService.setHistoryDir(dir);
  }

  m_pImpl->m_stopHudBtn = buildStopHudBtn(*m_pImpl);
  m_pImpl->m_infoBar = buildInfoBar();
  
  // Create and start table watcher
  m_pImpl->m_tableWatcher = buildTableWatcher(
    m_pImpl->m_watchedTablesLabel, 
    m_pImpl->m_playerIndicators, 
    m_pImpl->m_tableService);
  m_pImpl->m_tableWatcher->start();
  
  m_pImpl->m_mainWindow->end();
}

Gui::~Gui() = default;

/**
 * Displays the given msg in the info bar.
 */
void Gui::informUser(std::string_view msg) {
  ::informUser(*m_pImpl, msg);
}

/**
 * GUI entry point. will exit when all the windows are closed.
 */
/*[[nodiscard]]*/ int Gui::run() {
  LOG.debug<__func__>();
  m_pImpl->m_mainWindow->show();  // display the main window
  Fl::lock();  // enable multi-thread support
  return Fl::run();  // listen to awake() calls
}

// BUG : le stop HUD les cache mais ils réapparaissent
// BUG : filtrer plus la validation du répertoire de l'historique
// TODO : fonctionnalité de suppression des préférences utilisateurs
// TODO : sauvegarde des types de stat selectionnees
// TODO : remplacer le choix du répertoire d'historique à charger par un menu listant tous les répertoires connus
// TODO : remplacer les periodic task des StatProducer/StatReader par une requête exécutée si le fichier d'historique de la table change
/*
 * Position of the graphical elements on the main screen
 */
 // +---------------------------------------------------+--> X axis
 // |                 Main window title                 |
 // | ------------------------------------------------- |
 // | |                  menu bar                     | |
 // | ------------------------------------------------- |
 // | ------------------- ----------------------------  |
 // | |chose history dir| | histo file name           | |
 // | ------------------- ----------------------------- |
 // | ------------------------------------------------- |
 // | |              progress bar                     | |
 // | ------------------------------------------------- |
 // |                                        ---------- |
 // | Watching <table name>                  |Stop HUD| |
 // |                                        ---------- |
 // |---------------------------------------------------|
 // |                information bar                    |
 // +---------------------------------------------------+
 // |
 // V
 // Y axis