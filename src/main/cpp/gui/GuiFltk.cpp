#include "constants/TableConstants.hpp"
#include "entities/Seat.hpp"
#include "filesystem/FileUtils.hpp"
#include "gui/Gui.hpp"  // std::unique_ptr, std::make_unique
#include "gui/HistoryService.hpp"
#include "gui/MainWindowAppearance.hpp" // Surface::
#include "gui/MainWindowColor.hpp" // Color::
#include "gui/MainWindowLabel.hpp" // Label::
#include "gui/PlayerIndicator.hpp" // DragAndDropWindow, Fl_Double_Window
#include "gui/Position.hpp" // buildPlayerIndicatorPosition()
#include "gui/Preferences.hpp"
#include "gui/TableService.hpp"
#include "gui/TableWatcher.hpp"
#include "gui/WindowUtils.hpp" // mswindows::
#include "history/PokerSiteHistory.hpp"
#include "log/Logger.hpp" // CURRENT_FILE_NAME, fmt::*, Logger, StringLiteral
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp"
#include <gsl/gsl> // gsl::finally


#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027 )
#endif  // _MSC_VER

#include <FL/Enumerations.H> // Fl_Event
#include <FL/Fl.H> // Fl::awake
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H> // Fl_Group
#include <FL/Fl_Progress.H>
#include <FL/Fl_Tabs.H>
#include <FL/platform.H> // fl_xid

#if defined(_MSC_VER)  // end of specific msvc warnings removal
#  pragma warning(pop)
#endif  // _MSC_VER

#include <concepts> // requires
#include <ranges>
#include <unordered_map>

/*
* From http://www.fltk.org/doc-1.3/advanced.html#advanced_multithreading:
* - the main thread, i.e. the one running the run() function, draws widgets.
* - any other thread requesting widgets drawing must post a callback to the main thread using the
*   awake() function. It is possible to draw directly using lock()/unlock(), but the cb passing
*   method is less risky as lock()/unlock() blocks the main thread.
*/

static Logger& LOG() {
  static Logger logger { CURRENT_FILE_NAME };
  return logger;
}

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

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
  ~MyMainWindow() override = default;

  int handle(int e) override {
    // disable the 'close on Esc key' default behavior
    return ((Fl_Event::FL_SHORTCUT == e) and (FL_Escape == Fl::event_key()))
             ? 1
             : Fl_Double_Window::handle(e);
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
    , m_historyService { historyService } {
  }

  Implementation(const Implementation&) = delete;
  Implementation(Implementation&&) = delete;
  Implementation& operator=(const Implementation&) = delete;
  Implementation& operator=(Implementation&&) = delete;

  ~Implementation() {
    m_historyService.stopImportingHistory();
    if (m_tableWatcher) {
      m_tableWatcher->stop();
    }
    m_tableService.stopProducingStats();
    ThreadPool::stop();
  }
}; // struct Gui::Implementation

namespace {
  /* in anonymous namespace as type definitions can't be static */
  enum class [[nodiscard]] FileChoiceStatus : short { ok = 0, error = -1, cancel = 1 };

  /**
   * From Fl_get_system_colors.cxx :
   * skin = scheme = look and feel
  *
  * - "none" - This is the default look-n-feel which resembles old
  *            Windows (95/98/Me/NT/2000) and old GTK/KDE
  *
  * - "base" - This is an alias for "none"
  *
  * - "plastic" - This scheme is inspired by the Aqua user interface
  *               on macOS
  *
  * - "gtk+" - This scheme is inspired by the Red Hat Bluecurve theme
  *
  * - "gleam" - This scheme is inspired by the Clearlooks Glossy scheme.
  *             (Colin Jones and Edmanuel Torres).
  *
  * - "oxy" - This is a subset of Dmitrij K's oxy scheme (STR 2675, 3477)
  *
  *  If the given scheme name is unknown, the default scheme will be used.
  */
  namespace FltkSkin {
    /* from the int Fl::scheme(const char * s) documentation */
    [[maybe_unused]] constexpr std::string_view none { "none" };
    [[maybe_unused]] constexpr std::string_view base { "base" };
    [[maybe_unused]] constexpr std::string_view gleam { "gleam" };
    [[maybe_unused]] constexpr std::string_view gtkplus { "gtk+" };
    [[maybe_unused]] constexpr std::string_view plastic { "plastic" };
    [[maybe_unused]] constexpr std::string_view oxy { "oxy" };
  } // namespace FltkSkin

  template <typename T>
  [[nodiscard]] constexpr std::vector<T> toVector(std::span<const T> span) {
    return std::vector<T>(span.begin(), span.end());
  }

  using TablePlayerIndicators = std::array<std::unique_ptr<PlayerIndicator>, TableConstants::MAX_SEATS>;
  using TableWindowTitleToTablePlayerIndicators = std::unordered_map<std::string, TablePlayerIndicators>;

  /**
   * Defines the concept of a function taking no argument and returning void
   */
  template <typename F>
  concept VoidNullaryFunction = requires(F f) {
    // calling f returns void
    { f() } -> std::same_as<void>;
  } and std::is_invocable_v<F> and !std::is_invocable_v<F, int>;
  
  /**
   * Schedules a function to be executed by the main GUI thread during the next message handling cycle.
   * @param aTask the function to be executed
   */
  template <VoidNullaryFunction TASK>
  void scheduleUITask(TASK&& aTask) {
    using TaskType = std::decay_t<TASK>;
    Fl::awake([](void* hiddenTask) {
      const auto task = std::unique_ptr<TaskType>(static_cast<TaskType*>(hiddenTask));
      try {
        (*task)();
      }
      catch (const std::exception& e) {
        LOG().error<"Error in UI task: {}">(e.what());
      }
      catch (...) {
        LOG().error<"Unknown error in UI task">();
      }
      }, std::make_unique<TaskType>(std::forward<TASK>(aTask)).release());
  }

  void onImportProgress(Fl_Progress* progressBar) {
    scheduleUITask([pb = progressBar]() {
      pb->value(pb->value() + 1);
      pb->copy_label(fmt::format("{}/{}", limits::toInt(pb->value()), limits::toInt(pb->maximum())).c_str());
      });
  }

  void onSetNbFiles(Fl_Progress* progressBar, std::size_t nbFilesToLoad) {
    scheduleUITask([pb = progressBar, nbFilesToLoad]() {
      // as the progress bar boundaries are float values
      pb->maximum(static_cast<float>(nbFilesToLoad));
      });
  }

  void onImportDone(Fl_Button* chooseHistoDirBtn) {
    scheduleUITask([chb = chooseHistoDirBtn]() {
      chb->activate();
      });
  }

  /**
* Called by the GUI (button component) when the user clicks on the 'exit'
* button.
*/
// ReSharper disable once CppParameterMayBeConstPtrOrRef
// as this function has to be an Fl_Callback
  void exitCb(Fl_Widget* const mainWindow, void* hidden) {
    LOG().debug<__func__>();
    LOG().info<"Stopping the GUI.">();
    const auto& impl { *static_cast<Gui::Implementation*>(hidden) };

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

  template <typename WIDGET>
  [[nodiscard]] gsl::not_null<WIDGET*> createWidget(
    const phud::Rectangle& bounds, std::string_view label) {
    const auto& [x, y, w, h] { bounds };
    return new WIDGET(x, y, w, h, label.data());
  }

  template <typename WIDGET>
  [[nodiscard]] gsl::not_null<WIDGET*> createWidget(
    const phud::Rectangle& bounds, std::string_view label, auto setupFunction) {
    auto widget = createWidget<WIDGET>(bounds, label);
    setupFunction(widget);
    return widget;
  }

  template <typename WIDGET>
  [[nodiscard]] gsl::not_null<WIDGET*> createWidget(
    const phud::Rectangle& bounds, auto setupFunction) {
    const auto& [x, y, w, h] { bounds };
    auto widget = new WIDGET(x, y, w, h);
    setupFunction(widget);
    return widget;
  }

  [[nodiscard]] gsl::not_null<Fl_Menu_Bar*> buildMenuBar(Gui::Implementation* impl) {
    return createWidget<Fl_Menu_Bar>(MainWindow::Screen::menuBar,
      [impl](Fl_Menu_Bar* menu) {
        menu->add("&File/E&xit", 0, exitCb, impl);
        menu->box(FL_NO_BOX);
      });
  }

  [[nodiscard]] inline std::unique_ptr<Fl_Native_File_Chooser> buildDirectoryChooser(
    const Preferences& preferences) {
    const auto& startDir { preferences.getPreferredHistoDir() };
    auto pHistoryChoser = std::make_unique<Fl_Native_File_Chooser>();
    pHistoryChoser->title(MainWindow::Label::chooseHistoryDirectory.data());
    pHistoryChoser->type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    pHistoryChoser->directory(startDir.string().c_str());
    return pHistoryChoser;
  }

  /**
* Displays a message in the info bar, when the message is known at compile time.
*/
  template <const std::string_view& MSG>
  void informUser(Gui::Implementation& aSelf) {
    LOG().debug<__func__>();
    scheduleUITask([infoBar = aSelf.m_infoBar, msg = std::string(MSG)]() {
      infoBar->copy_label(msg.c_str());
      });
  }

  /**
  * Displays a message in the info bar.
  */
  void informUser(Gui::Implementation& aSelf, std::string_view aMsg) {
    LOG().debug<__func__>();
    scheduleUITask([infoBar = aSelf.m_infoBar, msg = std::string(aMsg)]() {
      infoBar->copy_label(msg.c_str());
      });
  }

  void updateTablePlayerIndicators(
    TablePlayerIndicators& playerIndicators,
    const phud::Rectangle& tablePosition,
    TableStatistics tableStatistics) {
    const auto heroSeat = tableStatistics.getHeroSeat();
    const auto& seats { tableStatistics.getSeats() };
    LOG().debug<"Processing {} seats for player indicators">(seats.size());

    for (const auto& seat : seats) {
      const auto& seatStr { tableSeat::toString(seat) };
      LOG().debug<"Checking seat {}">(seatStr);
      if (auto ps { tableStatistics.extractPlayerStatistics(seat) }; nullptr != ps) {
        LOG().debug<"Creating/updating indicator for player '{}' at seat {}">(ps->getPlayerName(), seatStr);
        const auto rotatedSeat = gui::rotateRelativeToHero(seat, heroSeat, tableStatistics.getMaxSeat());
        const auto pos = gui::buildPlayerIndicatorPosition(rotatedSeat, tableStatistics.getMaxSeat(), tablePosition);
        // update the PlayerIndicators with the latest stats.
        // -1 < seat < nbSeats, 1 < nbSeats < 11
        auto& playerIndicator = playerIndicators.at(tableSeat::toArrayIndex(seat));

        if (nullptr == playerIndicator) {
          LOG().debug<"Creating new PlayerIndicator for '{}'">(ps->getPlayerName());
          playerIndicator = std::make_unique<PlayerIndicator>(pos, ps->getPlayerName());
        }
        else {
          if (ps->getPlayerName() != playerIndicator->getPlayerName()) {
            LOG().debug<"Refreshing PlayerIndicator for '{}'">(ps->getPlayerName());
            playerIndicator->refresh(ps->getPlayerName());
          }
          playerIndicator->updateBasePosition(pos);
        }
        playerIndicator->setStats(*ps);
        mswindows::setWindowOnTopMost(fl_xid(&*playerIndicator));
        playerIndicator->show();
        LOG().debug<"PlayerIndicator shown for '{}'">(ps->getPlayerName());
      }
      else {
        LOG().debug<"No player statistics for seat {}, clearing indicator">(seatStr);
        // clear player indicators
        playerIndicators.at(tableSeat::toArrayIndex(seat)).reset();
      }
    }
  }

  void removeUselessPlayerIndicators(
    TableWindowTitleToTablePlayerIndicators& tableToPlayerIndicators,
    const std::span<const std::string> tableWindowTitles,
    TableService& tableService) {
    LOG().info<"Delete table indicators for removed table window(s)">();

    for (auto it = tableToPlayerIndicators.begin(); it != tableToPlayerIndicators.end();) {
      if (const auto title = it->first; !std::ranges::contains(tableWindowTitles, title)) {
        // Stop monitoring this table
        tableService.stopProducingStats();
        // Clear all player indicators for this table
        auto& playerIndicators = it->second;
        std::ranges::for_each(playerIndicators, [](auto& pi) { pi.reset(); });
        LOG().debug<"Removed player indicators for table window: {}">(title);
        it = tableToPlayerIndicators.erase(it);
      }
      else {
        ++it;
      }
    }
  }

  TableService::TableObserverCallback buildTableObserver(TableWindowTitleToTablePlayerIndicators& playerIndicators,
    std::string_view tableWindowTitle) {
    return [&playerIndicators, title = std::string(tableWindowTitle)](TableStatistics&& ts) {
      LOG().debug<"Observer called for table '{}'">(ts.getTable());
      LOG().debug<"About to schedule UI task for table '{}'">(ts.getTable());
      scheduleUITask([&playerIndicators, title, stats = std::move(ts)]() mutable {
        LOG().debug<"Scheduled UI task executing for table window '{}'">(title);

        if (const auto tableRect = mswindows::getTableWindowRectangle(title)) {
          LOG().debug<"Found table window '{}'">(title);
          // Update PlayerIndicators with real statistics
          if (playerIndicators.contains(title)) {
            LOG().debug<"Updating player indicators for table window '{}'">(title);
            updateTablePlayerIndicators(playerIndicators[title], *tableRect, std::move(stats));
          }
          else {
            LOG().warn<"Player indicators not found for table '{}'">(title);
          }
        }
        else {
          LOG().warn<"Could not get window rect for table window '{}'">(title);
        }
      });
    };
  }

  void updateUsefulPlayerIndicators(
    TableWindowTitleToTablePlayerIndicators& playerIndicators,
    const std::span<const std::string> tableWindowTitles,
    TableService& tableService) {
    LOG().info<"Create/Update table indicators for {} table(s)">(tableWindowTitles.size());
    const auto& tableTitleNotYetMonitored {
      tableWindowTitles
      | std::views::filter([&playerIndicators](const auto& title) { return !playerIndicators.contains(title); })
      | std::ranges::to<std::vector<std::string>>()
    };
    // Create entry if it doesn't exist
    std::ranges::for_each(tableTitleNotYetMonitored, [&playerIndicators, &tableService](const auto& title) {
      playerIndicators[title] = TablePlayerIndicators {};
      // Start monitoring this table for statistics
      const auto& observerCb { buildTableObserver(playerIndicators, title) };
      if (const auto& errorMsg { tableService.startProducingStats(title, observerCb) }; !errorMsg.empty()) {
        LOG().error<"Failed to start monitoring table window '{}': {}">(title, errorMsg);
      }
      });
  }

  /**
   * Called by the GUI event loop when the user chosed a valid history dir.
   * Starts the import process for a valid history directory.
   * Updates UI components and starts the background import.
   */
  void importDirAwakeCb(Fl_Progress* progressBar, Fl_Button* chooseHistoDirBtn, HistoryService& historyService,
    const fs::path& dir) {
    LOG().debug<__func__>();

    try {
      LOG().info<"The import directory '{}' is valid">(dir.string());

      // UI update - already running in UI thread, no need for scheduleUITask
      progressBar->activate();
      chooseHistoDirBtn->deactivate();

      // Start import through business service
      LOG().info<"About to call historyService.startImport">();
      historyService.importHistory(dir,
        // update the progress bar during the import
        [progressBar]() { onImportProgress(progressBar); },
        // when we know the number of files to import, setup the progress bar
        [progressBar](std::size_t nb) { onSetNbFiles(progressBar, nb); },
        // import completion callback
        [chooseHistoDirBtn]() { onImportDone(chooseHistoDirBtn); });
      LOG().info<"historyService.importHistory completed">();
    }
    catch (const std::exception& e) {
      LOG().error<"Exception in importDirAwakeCb: {}">(e.what());
    }
    catch (...) {
      LOG().error<"Unknown exception in importDirAwakeCb">();
    }
  }
} // anonymous namespace

namespace DirectoryChoiceHandler {
  void handleOk(std::string_view dirName, Gui::Implementation& self);
  void handleError(const Fl_Native_File_Chooser& chooser, Gui::Implementation& self);

  void handleOk(std::string_view dirName, Gui::Implementation& self) {
    const auto& dir { fs::path { dirName } };
    LOG().info<"the user chose to import the directory '{}'">(dir.string());

    if (self.m_historyService.isValidHistory(dir)) {
      self.m_preferences->saveHistoryDirectory(dir);
      self.m_historyService.setHistoryDir(dir); // Set the history directory in the service

      // Connect HistoryService and TableService
      if (const auto pokerSiteHistory { self.m_historyService.getPokerSiteHistory() }) {
        self.m_tableService.setPokerSiteHistory(pokerSiteHistory);
      }

      // Update directory label immediately (synchronously) to avoid pointer issues
      const auto& label = self.m_preferences->getHistoryDirectoryDisplayLabel();
      self.m_histoDirTextField->copy_label(label.c_str());

      // Schedule the rest of the UI updates and import
      scheduleUITask([pb = self.m_progressBar,
          chb = self.m_chooseHistoDirBtn,
          historyService = &self.m_historyService,
          dir]() {
          try {
            LOG().info<"About to start import process">();
            // Start import
            importDirAwakeCb(pb, chb, *historyService, dir);
            LOG().info<"Import process started successfully">();
          }
          catch (const std::exception& e) {
            LOG().error<"Exception in UI task: {}">(e.what());
          }
          catch (...) {
            LOG().error<"Unknown exception in UI task">();
          }
        });
    }
    else {
      LOG().info<"the chosen directory '{}' is not a valid history dir">(dir.string());
      informUser<MainWindow::Label::invalidChoice>(self);
    }
  }

  void handleError(const Fl_Native_File_Chooser& chooser, Gui::Implementation& self) {
    if (nullptr == chooser.errmsg()) {
      informUser<MainWindow::Label::fileChoiceError>(self);
    }
    else {
      informUser(self, chooser.errmsg());
    }
  }
} // namespace DirectoryChoiceHandler

static void handleDirectoryChoice(Gui::Implementation& self, const Fl_Native_File_Chooser& dirChoser, FileChoiceStatus fcs) {
    switch (fcs) {
    case FileChoiceStatus::ok:
      DirectoryChoiceHandler::handleOk(dirChoser.filename(), self);
      return;
    case FileChoiceStatus::error: [[unlikely]]
        DirectoryChoiceHandler::handleError(dirChoser, self);
      return;
    case FileChoiceStatus::cancel: /* nothing to do */ return;
    }
    std::unreachable();
}

[[nodiscard]] static gsl::not_null<Fl_Button*> buildChooseHistoDirBtn(
  Gui::Implementation& aSelf) {
  // Called by the GUI (button component) when the user clicks on the 'choose history directory' button.
  // Displays a directory chooser window.
  auto choseHistoDirCb = [](Fl_Widget*, void* hiddenSelf) {
    LOG().debug<"choseHistoDirCb">();
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
    informUser<MainWindow::Label::chooseHistoDirText>(self);
    const auto dirChoser = buildDirectoryChooser(*self.m_preferences);
    const auto choice = static_cast<FileChoiceStatus>(dirChoser->show());
    handleDirectoryChoice(self, *dirChoser, choice);
  };

  return createWidget<Fl_Button>(MainWindow::Screen::chooseHistoDirBtn,
                                 MainWindow::Label::chooseHistoDir,
                                 [&aSelf, choseHistoDirCb](Fl_Button* btn) {
                                   /* setup the button for choosing which history directory to import into database */
                                   btn->callback(choseHistoDirCb, &aSelf);
                                 });
}

[[nodiscard]] static gsl::not_null<Fl_Button*> buildStopHudBtn(Gui::Implementation& aSelf) {
  // Called by the GUI (button component) when the user clicks on the 'stop HUD'
  auto stopHudCb = [](Fl_Widget* button, void* hiddenSelf) {
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
    LOG().debug<"stopHudCb">();
    // Use business service to stop monitoring
    self.m_tableService.stopProducingStats();
    // kill PlayerIndicators
    for (auto& indicators : self.m_playerIndicators | std::views::values) {
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

[[nodiscard]] static gsl::not_null<Fl_Box*> buildHistoDirTextField(const Preferences& pref) {
  return createWidget<Fl_Box>(MainWindow::Screen::histoDirTextField, [&pref](Fl_Box* box) {
    /* setup the text box to display the chosen directory */
    const auto& label = pref.getHistoryDirectoryDisplayLabel();
    box->copy_label(label.c_str());
  });
}

[[nodiscard]] static gsl::not_null<Fl_Progress*> buildProgressBar() {
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

[[nodiscard]] static gsl::not_null<Fl_Box*> buildWatchedTablesLabel() {
  return createWidget<Fl_Box>(MainWindow::Screen::watchedTableLabel, MainWindow::Label::noPokerTableDetected,
                              [](Fl_Box* box) {
                                box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                              });
}

[[nodiscard]] static gsl::not_null<Fl_Box*> buildInfoBar() {
  return createWidget<Fl_Box>(MainWindow::Screen::infoBar, MainWindow::Label::WELCOME);
}

static std::string getWatchedTableLabel(std::span<const std::string> tableNames) {
  if (tableNames.empty()) {
    return MainWindow::Label::noPokerTableDetected.data();
  }
  if (tableNames.size() == 1) {
    return std::format(MainWindow::Label::watchingTable, tableNames.front());
  }
  return fmt::format(MainWindow::Label::watchingMultipleTables, tableNames.size());
}

[[nodiscard]] static std::unique_ptr<TableWatcher> buildTableWatcher(
  Fl_Box* pWatchedTableLabel,
  TableWindowTitleToTablePlayerIndicators& playerIndicators,
  TableService& tableService) {
  TableWatcher::TableWindowsDetectedCallback onTableWindowsDetectedCb {
    [pWatchedTableLabel, &playerIndicators, &tableService](std::span<const std::string> tableWindowTitles) {
      scheduleUITask(
        [pWatchedTableLabel, &playerIndicators, &tableService, twt = toVector(tableWindowTitles)]() {
          // Update label and player indicators
          const auto& label { getWatchedTableLabel(twt) };
          pWatchedTableLabel->copy_label(label.c_str());
          removeUselessPlayerIndicators(playerIndicators, twt, tableService);
          updateUsefulPlayerIndicators(playerIndicators, twt, tableService);
        });
    }
  };

  return std::make_unique<TableWatcher>(onTableWindowsDetectedCb);
}

static gsl::not_null<MyMainWindow*> buildMainWindow(Gui::Implementation* impl) {
  Fl::scheme(FltkSkin::gleam.data()); // select the look & feel
  Fl::visual(Fl_Mode::FL_DOUBLE | Fl_Mode::FL_INDEX); // enhance look where needed
  const auto [mx, my, mw, mh] { MainWindow::Screen::mainWindow };
  // ReSharper disable once CppDFAMemoryLeak
  const auto ret = new MyMainWindow(mx, my, mw, mh, MainWindow::Label::mainWindowTitle.data());
  const auto [x, y] { impl->m_preferences->getMainWindowPosition() };
  ret->position(x, y);
  ret->callback(exitCb, impl);
  return ret;
}

Gui::Gui(TableService& tableService, HistoryService& historyService)
  : m_pImpl { std::make_unique<Gui::Implementation>(tableService, historyService) } {
  LOG().debug<__func__>();
  m_pImpl->m_mainWindow = buildMainWindow(m_pImpl.get());
  m_pImpl->m_menuBar = buildMenuBar(m_pImpl.get());
  m_pImpl->m_chooseHistoDirBtn = buildChooseHistoDirBtn(*m_pImpl);
  m_pImpl->m_histoDirTextField = buildHistoDirTextField(*m_pImpl->m_preferences);
  m_pImpl->m_progressBar = buildProgressBar();
  m_pImpl->m_watchedTablesLabel = buildWatchedTablesLabel();

  if (const auto dir { m_pImpl->m_preferences->getPreferredHistoDir() }; PokerSiteHistory::isValidHistory(dir)) {
    m_pImpl->m_historyService.setHistoryDir(dir);
    m_pImpl->m_tableService.setHistoryDir(dir);
    // Connect HistoryService and TableService
    if (const auto h { m_pImpl->m_historyService.getPokerSiteHistory() }) {
      m_pImpl->m_tableService.setPokerSiteHistory(h);
    }
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
 * GUI entry point. will exit when all the windows are closed.
 */
/*[[nodiscard]]*/
int Gui::run() {
  LOG().debug<__func__>();
  m_pImpl->m_mainWindow->show(); // display the main window
  Fl::lock(); // enable multi-thread support
  return Fl::run(); // listen to awake() calls
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
