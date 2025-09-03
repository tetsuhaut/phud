#include "entities/Player.hpp"
#include "entities/Seat.hpp"
#include "gui/Gui.hpp"  // std::unique_ptr, std::make_unique
#include "gui/MainWindowAppearance.hpp" // Label::, Size::
#include "gui/PlayerIndicator.hpp" // DragAndDropWindow, Fl_Double_Window
#include "gui/Position.hpp" // buildPlayerIndicatorPosition()
#include "language/assert.hpp" // phudAssert
#include "language/Either.hpp" // ErrOrRes
#include "log/Logger.hpp" // CURRENT_FILE_NAME, fmt::*, Logger, StringLiteral
#include "mainLib/AppInterface.hpp"  // std::string, std::vector, std::filesystem::path, fs::isDir
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
 * The function to be executed must take no argument
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

[[nodiscard]] static inline std::unique_ptr<Fl_Preferences> buildPreferences() {
  /* preference name, value, default value */
  return std::make_unique<Fl_Preferences>(Fl_Preferences::USER,
    ProgramInfos::APP_SHORT_NAME.data(), ProgramInfos::APP_SHORT_NAME.data());
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

struct TableChooser;

struct [[nodiscard]] Gui::Implementation final {
  AppInterface& m_app;
  Fl_Group* m_winamaxGroup { nullptr };
  Fl_Group* m_pmuGroup { nullptr };
  Fl_Tabs* m_tabs { nullptr };
  MyMainWindow* m_mainWindow { nullptr };
  std::unique_ptr<Fl_Preferences> m_preferences { buildPreferences() };
  Fl_Button* m_chooseHistoDirBtn { nullptr };
  Fl_Button* m_chooseTableBtn { nullptr };
  Fl_Button* m_stopHudBtn { nullptr };
  Fl_Box* m_histoDirTextField { nullptr };
  Fl_Progress* m_progressBar { nullptr };
  Fl_Box* m_infoBar { nullptr };
  Fl_Menu_Bar* m_menuBar { nullptr };
  std::unique_ptr<TableChooser> m_tableChooser {};
  std::shared_ptr<std::array<std::unique_ptr<PlayerIndicator>, 10>> m_playerIndicators { std::make_shared<std::array<std::unique_ptr<PlayerIndicator>, 10>>() };

  explicit Implementation(AppInterface& app) : m_app { app } {}
  Implementation(const Implementation&) = delete;
  Implementation(Implementation&&) = delete;
  Implementation& operator=(const Implementation&) = delete;
  Implementation& operator=(Implementation&&) = delete;
  ~Implementation() { ThreadPool::stop(); }
}; // struct Gui::Implementation

[[nodiscard]] static inline std::string getLastErrorMessageFromOS() {
  const auto localeId { LocaleNameToLCID(LOCALE_NAME_SYSTEM_DEFAULT, 0) };
  char err[MAX_PATH + 1] { '\0' };
  const auto size { FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), localeId, &err[0], MAX_PATH, nullptr) };
  err[MAX_PATH] = '\0';
  return 0 == size ? "Failed to retrieve error message from system" : std::string(&err[0]);
}

template <typename T>
[[nodiscard]] constexpr static std::unique_ptr<T> mkWidget(const phud::Rectangle& r,
  std::string_view label) {
  return std::make_unique<T>(r.x, r.y, r.w, r.h, label.data());
}

template <typename T>
[[nodiscard]] constexpr static std::unique_ptr<T> mkWidget(const phud::Rectangle& r) {
  return std::make_unique<T>(r.x, r.y, r.w, r.h);
}

[[nodiscard]] static inline std::string getExecutableName(const HWND window) {
  LOG.debug<__func__>();

  if (nullptr == window) { return ""; }

  DWORD pid;
  GetWindowThreadProcessId(window, &pid);
  const auto myProcessHandle { OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid) };
  phudAssert(nullptr != myProcessHandle, getLastErrorMessageFromOS().c_str());
  auto _ { gsl::finally([myProcessHandle] { CloseHandle(myProcessHandle); }) };
  char process[MAX_PATH + 1] { '\0' };

  if (const auto nbChars { GetModuleFileNameEx(myProcessHandle, nullptr, &process[0], MAX_PATH) }; 0 != nbChars) {
    process[MAX_PATH] = '\0';
    return std::string(& process[0]);
  }
  LOG.error<"Can't retrieve the executable name: {}">(getLastErrorMessageFromOS());
  return "";
}

/**
* Converts a RECT object from the Windows API into a phud::Rectangle object
*/
[[nodiscard]] constexpr static phud::Rectangle toRectangle(const RECT& r) noexcept {
  return { r.left, r.top, r.right - r.left, r.bottom - r.top };
}

[[nodiscard]] static inline fs::path getPreferredHistoDir(Fl_Preferences& pref) {
  char dir[MAX_PATH + 1] { '\0' };
  pref.get(MainWindow::Label::preferencesKeyChosenDir.data(), &dir[0], "", MAX_PATH);
  dir[MAX_PATH] = '\0';
  const auto& pathDir { fs::path(dir) };
  return pf::isDir(pathDir) ? pathDir : "";
}

[[nodiscard]] /*static*/ inline std::unique_ptr<Fl_Native_File_Chooser> buildDirectoryChooser(
  Fl_Preferences& preferences) {
  const auto& startDir { getPreferredHistoDir(preferences) };
  auto pHistoryChoser { std::make_unique<Fl_Native_File_Chooser>() };
  pHistoryChoser->title(MainWindow::Label::chooseHistoryDirectory.data());
  pHistoryChoser->type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
  pHistoryChoser->directory(startDir.string().c_str());
  return pHistoryChoser;
}

/**
* Displays a literal message in the info bar.
*/
template<StringLiteral MSG>
static inline void informUser(Gui::Implementation& aSelf) {
  LOG.debug<__func__>();
  scheduleUITask([infoBar = aSelf.m_infoBar, msg = std::string(MSG.value)]() {
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

/**
 * Updates the statistics of all the PlayerIndicators of the table.
 * Called periodically by another thread.
 */
static inline void updateTableAwakeCb(std::shared_ptr<std::array<std::unique_ptr<PlayerIndicator>, 10>> playerIndicators, const phud::Rectangle& tablePosition,
  TableStatistics tableStatistics) {
  const auto heroSeat { tableStatistics.getHeroSeat() };
  const auto& seats { tableStatistics.getSeats() };

  for (const auto& seat : seats) {
    auto ps { tableStatistics.extractPlayerStatistics(seat) };
    // clear player indicators
    if (nullptr == ps) {
      scheduleUITask([playerIndicators, seat]() { 
        playerIndicators->at(tableSeat::toArrayIndex(seat)).reset();
      });
    }
    else {
      const auto& pos { buildPlayerIndicatorPosition(seat, heroSeat, tableStatistics.getMaxSeat(), tablePosition) };
      // update the PlayerIndicators with the latest stats.
      // -1 < seat < nbSeats, 1 < nbSeats < 11
      scheduleUITask([playerIndicators, seat, pos, ps = std::move(ps)]() {
        auto& playerIndicator { playerIndicators->at(tableSeat::toArrayIndex(seat)) };

        if (nullptr == playerIndicator) {
          playerIndicator = std::make_unique<PlayerIndicator>(pos, ps->getPlayerName());
        }
        else if (ps->getPlayerName() != playerIndicator->getPlayerName()) {
          playerIndicator->refresh(ps->getPlayerName());
        }
        playerIndicator->setStats(*ps);
        setWindowOnTopMost(*playerIndicator);
        playerIndicator->show();
        });
    }
  }
}

static inline bool isLabelChooseTable(const Gui::Implementation& self) {
  return 0 == MainWindow::Label::chooseTable.compare(self.m_chooseTableBtn->label());
}

static inline void setChooseTableButtonLabelToChoose(Fl_Button* chooseTableBtn) {
  chooseTableBtn->label(MainWindow::Label::chooseTable.data());
}

using ErrorOrRectangleAndName = ErrOrRes<std::pair<phud::Rectangle, std::string>>;
/** Gets the current window absolute position,
 * ensures the process owning the window is the poker app,
 * gets the name of the window this position belongs to.
 */
[[nodiscard]] static inline ErrorOrRectangleAndName getWindowRectangleAndName(const AppInterface& app, int x, int y) {
  LOG.debug<__func__>();
  const auto& myWindowHandle { WindowFromPoint({x, y}) };

  if (nullptr == myWindowHandle) { return ErrorOrRectangleAndName::err<"No window at the given position">(); }

  if (!app.isPokerApp(getExecutableName(myWindowHandle))) { return ErrorOrRectangleAndName::err<"The chosen window is not a poker table.">(); }

  if (RECT r; 0 != GetWindowRect(myWindowHandle, &r)) {
    char tableName[MAX_PATH + 1] { '\0' };
    GetWindowText(myWindowHandle, &tableName[0], MAX_PATH);
    tableName[MAX_PATH] = '\0';
    return ErrorOrRectangleAndName::res({ toRectangle(r), tableName });
  }

  return ErrorOrRectangleAndName::err<"Could not get the chosen window handle.">();
}



/**
 * Called by the GUI (table chooser widget) when the user drags the table
 * chooser widget above a window and drops it.
 */
static inline void tableChooserCb(AppInterface& app, 
                                 std::unique_ptr<TableChooser>& tableChooser,
                                 Fl_Button* chooseTableBtn,
                                 std::shared_ptr<std::array<std::unique_ptr<PlayerIndicator>, 10>> playerIndicators,
                                 int x, int y) {
  LOG.debug<__func__>();
  const auto& errorOrResult { getWindowRectangleAndName(app, x, y) };

  if (errorOrResult.isErr()) {
    LOG.info(errorOrResult.getErr());
    // Cannot call informUser here without infoBar - would need to be passed as parameter
    LOG.error(errorOrResult.getErr());
  }
  else {
    const auto& [tablePosition, tableName] { errorOrResult.getRes() };
    LOG.info<"The chosen poker table is '{}'">(tableName);
    scheduleUITask([tableChooser = std::move(tableChooser), chooseTableBtn]() mutable {
      tableChooser = nullptr;
      setChooseTableButtonLabelToChoose(chooseTableBtn);
      });
    LOG.info<"Starting consuming stats.">();
    const auto statObserver = [playerIndicators, tablePosition](TableStatistics&& ts) {
      // to read table statistics, we need to extract data from it -> non const
      scheduleUITask([playerIndicators, tablePosition, ts = std::move(ts)]() mutable {
          updateTableAwakeCb(playerIndicators, tablePosition, std::move(ts));
        });
      };

    if (const auto& errMsg { app.startProducingStats(tableName, statObserver) }; !errMsg.empty()) {
      LOG.error(errMsg);
    }
    else {
      // Cannot activate stopHudBtn here without passing it as parameter
      LOG.info<"Stats production started successfully">();
    }
  }
}

/*
 * A table chooser, i.e. a dragndrop window that calls tableChooserCb when dropped somewhere.
 */
struct [[nodiscard]] TableChooser final : DragAndDropWindow {
  TableChooser(Gui::Implementation & self, std::string_view label)
    : DragAndDropWindow(MainWindow::Surface::getTableChooserRectangle(self.m_mainWindow->x(),
                        self.m_mainWindow->y(), self.m_chooseTableBtn->x()), label, 
                        [app = &self.m_app, 
                         tableChooser = &self.m_tableChooser,
                         chooseTableBtn = self.m_chooseTableBtn,
                         playerIndicators = self.m_playerIndicators](int x, int y) {
        tableChooserCb(*app, *tableChooser, chooseTableBtn, playerIndicators, x, y);
    }) {}
}; // struct TableChooser

/**
* @returns the table chooser widget
*/
[[nodiscard]] static inline std::unique_ptr<TableChooser> buildTableChooser(
  Gui::Implementation& self, std::string_view label) {
  LOG.debug<__func__>();
  auto ret { std::make_unique<TableChooser>(self, label) };
  ret->color(FL_BLUE);
  ret->clear_border();
  setWindowOnTopMost(*ret);
  ret->show();
  return ret;
}

template<typename T> requires(std::same_as<T, const char*> or std::same_as<T, int>)
static inline void saveToPreferences(Fl_Preferences& pref, std::string_view name, T&& value) {
  if (0 == pref.set(name.data(), std::forward<decltype(value)>(value))) {
    LOG.error<"Couldn't save '{}' into the preferences repository.">(name);
  }
}

/**
* Called by the GUI (button component) when the user clicks on the 'exit'
* button.
*/
static inline void exitCb(Fl_Widget* const mainWindow, void* hidden) {
  LOG.debug<__func__>();
  LOG.info<"Stopping the GUI.">();
  auto& preferences { *static_cast<Fl_Preferences*>(hidden) };
  /* save the main window position */
  saveToPreferences(preferences, MainWindow::Label::x, mainWindow->x());
  saveToPreferences(preferences, MainWindow::Label::y, mainWindow->y());
  saveToPreferences(preferences, MainWindow::Label::width, mainWindow->w());
  saveToPreferences(preferences, MainWindow::Label::height, mainWindow->h());

  /* close all subwindows */
  while (Fl::first_window()) { 
    Fl::first_window()->hide();
    Fl::check(); // Force FLTK to treat events
  }
}

/**
 * Called by the GUI event loop when the user chosed a valid history dir.
 * Starts the import process for a valid history directory.
 * Updates UI components and starts the background import.
 */
static inline void importDirAwakeCb(Fl_Box* histoDirTextField, Fl_Progress* progressBar, Fl_Button* chooseTableBtn, AppInterface& app, const fs::path& dir) {
  LOG.debug<__func__>();
  const auto& historyDir { dir.filename().string() };
  LOG.info<"The import directory '{}' is valid">(historyDir);
  // UI update
  scheduleUITask([histoDirTextField, historyDir, progressBar]() {
    // the string is copied in the widget internal buffer
    histoDirTextField->copy_label(historyDir.c_str());
    progressBar->activate();
  });
  LOG.info<"importing history">();
  // Called when the history loader (another thread) notifies the loading has made progress.
  auto incrementCb = [progressBar]() {
    scheduleUITask([pb = progressBar]() {
      pb->value(pb->value() + 1);
      pb->copy_label(fmt::format("{}/{}", limits::toInt(pb->value()), limits::toInt(pb->maximum())).c_str());
    });
  };
  // called when the history loader (another thread) notifies of how many file will be loaded.
  auto setNbFilesCb = [progressBar](std::size_t nbFilesToLoad) {
    scheduleUITask([pb = progressBar, nbFilesToLoad]() { pb->maximum(static_cast<float>(nbFilesToLoad)); });
  };
  // called when the history loader (another thread) notifies the loading is done.
  auto doneCb = [chooseTableBtn]() {
    scheduleUITask([chooseTableBtn]() { chooseTableBtn->activate(); });
  };
  // start the import
  app.importHistory(dir, incrementCb, setNbFilesCb, doneCb);
}

static inline std::pair<int, int> getMainWindowPosition(Fl_Preferences& preferences) {
  /* get the previous width and height from preferences, if any */
  int width, height;
  preferences.get(MainWindow::Label::width.data(), width, MainWindow::Surface::mainWindow.w);
  preferences.get(MainWindow::Label::height.data(), height,
    MainWindow::Surface::mainWindow.h);
  /* compute the center position */
  int dummyX, dummyY, screenWidth, screenHeight;
  Fl::screen_xywh(dummyX, dummyY, screenWidth, screenHeight);
  const auto initX { (screenWidth - width) / 2 };
  const auto initY { (screenHeight - height) / 2 };
  /* get the previous position from preferences. if none, use the center position */
  int x, y;
  preferences.get(MainWindow::Label::x.data(), x, initX);
  preferences.get(MainWindow::Label::y.data(), y, initY);
  return { x, y };
}

template<typename WIDGET>
[[nodiscard]] static inline gsl::not_null<WIDGET*> createWidget(
  const phud::Rectangle& bounds, std::string_view label) {
  const auto [x, y, w, h] { bounds };
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
  const auto [x, y, w, h] { bounds };
  auto widget = new WIDGET(x, y, w, h);
  setupFunction(widget);
  return widget;
}

[[nodiscard]] static inline gsl::not_null<Fl_Menu_Bar*> buildMenuBar(Fl_Preferences* preferences) {
  return createWidget<Fl_Menu_Bar>(MainWindow::Surface::menuBar, [preferences](Fl_Menu_Bar* menu) {
    menu->add("&File/E&xit", 0, exitCb, preferences);
    menu->box(FL_NO_BOX);
  });
}

namespace DirectoryChoiceHandler {
  void handleOk(std::string_view dirName, Gui::Implementation& self) {
    const auto dir { fs::path { dirName } };
    LOG.info<"the user chose to import the directory '{}'">(dir.string());

    if (AppInterface::isValidHistory(dir)) {
      saveToPreferences(*self.m_preferences, MainWindow::Label::preferencesKeyChosenDir,
        dir.string().c_str()); // to get const char*
      scheduleUITask([hdtf = self.m_histoDirTextField, pb = self.m_progressBar, ctb = self.m_chooseTableBtn, app = &self.m_app, dir = std::move(dir)]() {
        importDirAwakeCb(hdtf, pb, ctb, *app, dir);
        });
    }
    else {
      LOG.info<"the chosen directory '{}' is not a valid history dir">(dir.string());
      informUser(self, MainWindow::Label::invalidChoice);
    }
  }

  void handleError(Fl_Native_File_Chooser* chooser, Gui::Implementation& self) {
    if (nullptr == chooser->errmsg()) { 
      informUser<"File choice error">(self); 
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
    informUser<"">(self);
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

  return createWidget<Fl_Button>(MainWindow::Surface::chooseHistoDirBtn, 
                                 MainWindow::Label::chooseHistoDir, 
                                 [&aSelf, choseHistoDirCb](Fl_Button* btn) {
    /* setup the button for choosing which history directory to import into database */
    btn->callback(choseHistoDirCb, &aSelf);
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildChooseTableBtn(Gui::Implementation& aSelf) {
  
  // Called by the GUI (button component) when the user clicks on the 'choose table' button.
  auto chooseTableCb = [](Fl_Widget* button, void* hiddenSelf) {
    LOG.debug<__func__>();
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };

    if (isLabelChooseTable(self)) {
      button->label(MainWindow::Label::cancelTableChoice.data());
      self.m_tableChooser = buildTableChooser(self, MainWindow::Label::chooseTable);
    }
    else {
      setChooseTableButtonLabelToChoose(self.m_chooseTableBtn);
      self.m_tableChooser = nullptr;
    }
  };

  return createWidget<Fl_Button>(MainWindow::Surface::chooseTableBtn, 
                                 MainWindow::Label::chooseTable, 
                                 [&aSelf, chooseTableCb](Fl_Button* btn) {
    /* setup the table chooser */
    btn->callback(chooseTableCb, &aSelf);
    /* btn->deactivate(); */
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildStopHudBtn(Gui::Implementation& aSelf) {
  
  // Called by the GUI (button component) when the user clicks on the 'stop HUD'
  auto stopHudCb = [](Fl_Widget* button, void* hiddenSelf) {
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
    LOG.debug<__func__>();
    // kill stats producer/receiver
    self.m_app.stopProducingStats();
    // kill PlayerIndicators
    std::ranges::for_each(*self.m_playerIndicators, [](auto& pi) { pi.reset(); });
    button->deactivate();
    informUser<"no player indicator to display">(self);
    Fl::redraw();
  };

  return createWidget<Fl_Button>(MainWindow::Surface::stopHudBtn, 
                                 MainWindow::Label::stopHud, 
                                 [&aSelf, stopHudCb](Fl_Button* btn) {
    btn->callback(stopHudCb, &aSelf);
    btn->deactivate();
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildHistoDirTextField(Fl_Preferences& pref) {
  return createWidget<Fl_Box>(MainWindow::Surface::histoDirTextField, [&pref](Fl_Box* box) {
    /* setup the text box to display the chosen directory */
    box->copy_label(pf::isDir(getPreferredHistoDir(pref)) ?
      MainWindow::Label::HAND_HISTORY_EXISTS.data() :
      MainWindow::Label::CHOOSE_HAND_HISTORY_DIRECTORY.data());
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Progress*> buildProgressBar() {
  return createWidget<Fl_Progress>(MainWindow::Surface::progressBar, [](Fl_Progress* progress) {
    progress->color(MainWindow::Color::progressBarBackgroundColor);
    progress->selection_color(MainWindow::Color::progressBarForegroundColor);
    progress->labelcolor(MainWindow::Color::progressBarTextColor);
    progress->minimum(0);
    progress->maximum(1);
    progress->deactivate();
    //progress->box(FL_NO_BOX);
  });
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildInfoBar() {
  return createWidget<Fl_Box>(MainWindow::Surface::infoBar, MainWindow::Label::WELCOME);
}

static inline gsl::not_null<MyMainWindow*> buildMainWindow(Fl_Preferences* preferences) {
  Fl::scheme(FltkSkin::gleam.data());  // select the look & feel
  Fl::visual(Fl_Mode::FL_DOUBLE | Fl_Mode::FL_INDEX);  // enhance look where needed
  const auto [mx, my, mw, mh] { MainWindow::Surface::mainWindow };
  auto ret { new MyMainWindow(mx, my, mw, mh, MainWindow::Label::mainWindowTitle.data()) };
  const auto [x, y] { getMainWindowPosition(*preferences) };
  ret->position(x, y);
  ret->callback(exitCb, preferences);
  return ret;
}

Gui::Gui(AppInterface& app)
  : m_pImpl { std::make_unique<Gui::Implementation>(app) } {
  LOG.debug<__func__>();
  m_pImpl->m_mainWindow = buildMainWindow(m_pImpl->m_preferences.get());
  m_pImpl->m_menuBar = buildMenuBar(m_pImpl->m_preferences.get());
  m_pImpl->m_chooseHistoDirBtn = buildChooseHistoDirBtn(*m_pImpl);
  m_pImpl->m_histoDirTextField = buildHistoDirTextField(*m_pImpl->m_preferences);
  m_pImpl->m_progressBar = buildProgressBar();
  m_pImpl->m_chooseTableBtn = buildChooseTableBtn(*m_pImpl);

  if (const auto dir { getPreferredHistoDir(*m_pImpl->m_preferences) }; !dir.empty() and pf::isDir(dir)) {
    m_pImpl->m_chooseTableBtn->activate();
    m_pImpl->m_app.setHistoryDir(dir);
  }

  m_pImpl->m_stopHudBtn = buildStopHudBtn(*m_pImpl);
  m_pImpl->m_infoBar = buildInfoBar();
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
 * GUI entry point. will exit if all the window are closed.
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
// +------------------------> X
// | ---------------------|
// | GUI                  |
// |----------------------|
// |     info bar         |
// +----------------------+
// |
// V
// Y