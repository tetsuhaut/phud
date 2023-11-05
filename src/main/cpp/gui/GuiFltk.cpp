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
#include <frozen/unordered_map.h>
#include <gsl/gsl> // gsl::finally

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4514 4625 4626 4820 5026 5027 )
#elif defined(__MINGW32__) // removal of specific gcc warnings due to FLTK
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Weffc++"
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
#elif defined(__MINGW32__)
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

#include <windows.h> // RECT, GetWindowThreadProcessId, MAX_PATH, WindowFromPoint, GetWindowRect, GetWindowText, SetWindowPos
#include <psapi.h>  // GetModuleFileNameEx
#include <concepts>

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
  }; // namespace FltkSkin

}; // anonymous namespace

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
  std::array<std::unique_ptr<PlayerIndicator>, 10> m_playerIndicators {};

  explicit Implementation(AppInterface& app) : m_app { app } {}
  Implementation(const Implementation&) = delete;
  Implementation(Implementation&&) = delete;
  Implementation& operator=(const Implementation&) = delete;
  Implementation& operator=(Implementation&&) = delete;
  ~Implementation() = default;
}; // struct Gui::Implementation

[[nodiscard]] static inline std::string getLastErrorMessageFromOS() {
  char err[MAX_PATH + 1] { '\0' };
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &err[0], MAX_PATH,
    nullptr);
  return &err[0];
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
  const auto msg { getLastErrorMessageFromOS() };
  phudAssert(nullptr != myProcessHandle, msg.c_str());
  auto _ { gsl::finally([&] { CloseHandle(myProcessHandle); }) };
  char process[MAX_PATH + 1] { '\0' };
  GetModuleFileNameEx(myProcessHandle, nullptr, &process[0], MAX_PATH);
  return &process[0];
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
  const auto& pathDir { fs::path(dir) };
  return phud::filesystem::isDir(pathDir) ? pathDir : "";
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
  Fl::awake([](void* hiddenSelf) {
    auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
    self.m_infoBar->copy_label(MSG.value);
    }, &aSelf);
}

struct [[nodiscard]] InformUserArgs final {
  Gui::Implementation& m_self;
  std::string m_msg;

  InformUserArgs(Gui::Implementation& self, std::string_view msg)
    : m_self(self),
    m_msg(msg) {}
};

/**
* Displays a message in the info bar.
*/
static inline void informUser(Gui::Implementation& aSelf, std::string_view aMsg) {
  LOG.debug<__func__>();
  Fl::awake([](void* hidden) {
    auto pInformUser { std::unique_ptr<InformUserArgs>(static_cast<InformUserArgs*>(hidden)) };
    pInformUser->m_self.m_infoBar->copy_label(pInformUser->m_msg.data());
    }, new InformUserArgs(aSelf, aMsg));
}

/**
* Called by the GUI (button component) when the user clicks on the 'stop HUD'
* button.
*/
static inline void stopHudCb(Fl_Widget* button, void* hiddenSelf) {
  auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
  LOG.debug<__func__>();
  // kill stats producer/receiver
  self.m_app.stopProducingStats();
  // kill PlayerIndicators
  std::ranges::for_each(self.m_playerIndicators, [](auto& pi) { pi.reset(); });
  button->deactivate();
  informUser<"no player indicator to display">(self);
  Fl::redraw();
}

static inline void setWindowOnTopMost(const Fl_Window& above) {
  /* from https://www.fltk.org/newsgroups.php?s39452+gfltk.general+v39464 */
  SetWindowPos(fl_xid(&above), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
}

std::unique_ptr<PlayerIndicator>& getPlayerIndicator(Gui::Implementation& self, Seat seat) {
  return self.m_playerIndicators.at(tableSeat::toArrayIndex(seat));
}

struct [[nodiscard]] UpdatePlayerIndicatorsArgs final {
  Gui::Implementation& m_self;
  std::unique_ptr<PlayerStatistics> m_ps;
  std::pair<int, int> m_pos;
  Seat m_seat;

  UpdatePlayerIndicatorsArgs(Gui::Implementation& self, Seat seat, std::pair<int, int> pos,
    std::unique_ptr<PlayerStatistics> ps) :
    m_self(self),
    m_ps(std::move(ps)),
    m_pos(pos),
    m_seat(seat)
  {}
};

/**
* Called by the stat watcher (another thread) to notify of new stats. Will
* update the PlayerIndicators with the latest stats.
* tablePosition is copied
* -1 < seat < nbSeats
* 1 < nbSeats < 11
*/
static inline void updatePlayerIndicatorsAwakeCb(void* hidden) {
  const auto args { std::move(static_cast<UpdatePlayerIndicatorsArgs*>(hidden)) };
  phudAssert(nullptr != args->m_ps, "ps nullptr dans updatePlayerIndicatorsAwakeCb");
  auto& playerIndicator { getPlayerIndicator(args->m_self, args->m_seat) };

  if (nullptr == playerIndicator) {
    playerIndicator = std::make_unique<PlayerIndicator>(args->m_pos, args->m_ps->getPlayerName());
  }
  else if (args->m_ps->getPlayerName() != playerIndicator->getPlayerName()) {
    playerIndicator->refresh(args->m_ps->getPlayerName());
  }

  playerIndicator->setStats(*args->m_ps);
  setWindowOnTopMost(*playerIndicator);
  playerIndicator->show();
}

struct [[nodiscard]] ResetPlayerIndicatorArgs final {
  Gui::Implementation& m_self;
  Seat m_seat;

  ResetPlayerIndicatorArgs(Gui::Implementation& self, Seat seat)
    : m_self(self),
    m_seat(seat) {}
};

static inline void resetPlayerIndicatorsAwakeCb(void* hidden) {
  LOG.debug<__func__>();
  auto rpi { std::unique_ptr<ResetPlayerIndicatorArgs>(static_cast<ResetPlayerIndicatorArgs*>(hidden)) };
  getPlayerIndicator(rpi->m_self, rpi->m_seat).reset();
}

struct [[nodiscard]] UpdateTableArgs final {
  Gui::Implementation& m_self;
  TableStatistics m_tableStatistics;
  phud::Rectangle m_tablePosition;

  UpdateTableArgs(Gui::Implementation& self, phud::Rectangle tablePosition,
    TableStatistics tableStatistics)
    : m_self(self),
    m_tableStatistics(std::move(tableStatistics)),
    m_tablePosition(tablePosition)
     {}
};

/**
 * Updates the statistics of all the PlayerIndicators of the table.
 * Called periodically by another thread.
 */
static inline void updateTableAwakeCb(void* hidden) {
  auto args { std::unique_ptr<UpdateTableArgs>(static_cast<UpdateTableArgs*>(hidden)) };
  const auto heroSeat { args->m_tableStatistics.getHeroSeat() };
  const auto& seats { args->m_tableStatistics.getSeats() };

  for (const auto& seat : seats) {
    auto ps { args->m_tableStatistics.extractPlayerStatistics(seat) };

    if (nullptr == ps) {
      Fl::awake(resetPlayerIndicatorsAwakeCb, new ResetPlayerIndicatorArgs(args->m_self, seat));
    }
    else {
      const auto& pos {
        (Seat::seatUnknown == heroSeat) ?
        buildPlayerIndicatorPosition(seat, args->m_tableStatistics.getMaxSeat(), args->m_tablePosition)
        : buildPlayerIndicatorPosition(seat, heroSeat, args->m_tableStatistics.getMaxSeat(), args->m_tablePosition)
      };
      Fl::awake(updatePlayerIndicatorsAwakeCb, new UpdatePlayerIndicatorsArgs(args->m_self, seat, pos,
        std::move(ps)));
    }
  }
}

static inline bool isLabelChooseTable(const Gui::Implementation& self) {
  return 0 == MainWindow::Label::chooseTable.compare(self.m_chooseTableBtn->label());
}

static inline void setChooseTableButtonLabelToChoose(Gui::Implementation& self) {
  self.m_chooseTableBtn->label(MainWindow::Label::chooseTable.data());
}

using ErrorOrRectangleAndName = ErrOrRes<std::pair<phud::Rectangle, std::string>>;
/** Gets the current window absolute position,
 * ensures the process owning the window is the poker app,
 * gets the name of the window this position belongs to.
 */
[[nodiscard]] static inline ErrorOrRectangleAndName getWindowRectangleAndName(int x, int y) {
  LOG.debug<__func__>();
  const auto& myWindowHandle { WindowFromPoint({x, y}) };

  if (nullptr == myWindowHandle) { return ErrorOrRectangleAndName::err<"No window at the given position">(); }

  if (!AppInterface::isPokerApp(getExecutableName(myWindowHandle))) { return ErrorOrRectangleAndName::err<"The chosen window is not a poker table.">(); }

  if (RECT r; 0 != GetWindowRect(myWindowHandle, &r)) {
    char tableName[MAX_PATH + 1] { '\0' };
    GetWindowText(myWindowHandle, &tableName[0], MAX_PATH);
    return ErrorOrRectangleAndName::res({ toRectangle(r), tableName });
  }

  return ErrorOrRectangleAndName::err<"Could not get the chosen window handle.">();
}

/**
 * Called by the GUI (table chooser widget) when the user drags the table
 * chooser widget above a window and drops it.
 */
static inline void tableChooserCb(Gui::Implementation& self, int x, int y) {
  LOG.debug<__func__>();
  const auto& errorOrResult { getWindowRectangleAndName(x, y) };

  if (errorOrResult.isErr()) {
    LOG.info(errorOrResult.getErr());
    informUser(self, errorOrResult.getErr());
  }
  else {
    const auto& [tablePosition, tableName] { errorOrResult.getRes() };
    informUser(self, fmt::format("The chosen poker table is '{}'", tableName));
    self.m_tableChooser = nullptr;
    setChooseTableButtonLabelToChoose(self);
    LOG.info<"Starting consuming stats.">();

    if (const auto& errMsg { self.m_app.startProducingStats(tableName,
    [&self, tablePosition](TableStatistics&& ts) {
    Fl::awake(updateTableAwakeCb, new UpdateTableArgs(self, tablePosition, std::move(ts)));
    }) }; !errMsg.empty()) {
      informUser(self, errMsg);
    }
    else { self.m_stopHudBtn->activate(); }
  }
}

/*
 * A table chooser, i.e. a dragndrop window that calls tableChooserCb when dropped somewhere.
 */
struct [[nodiscard]] TableChooser final : DragAndDropWindow {
  TableChooser(Gui::Implementation & self, std::string_view label)
    : DragAndDropWindow(MainWindow::Surface::getTableChooserRectangle(self.m_mainWindow->x(),
                        self.m_mainWindow->y(), self.m_chooseTableBtn->x()), label, [&self](int x, int y) {
    tableChooserCb(self, x, y);
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

/**
* Called by the GUI (button component) when the user clicks on the 'choose table'
* button.
*/
static inline void chooseTableCb(Fl_Widget* button, void* hiddenSelf) {
  LOG.debug<__func__>();
  auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };

  if (isLabelChooseTable(self)) {
    button->label(MainWindow::Label::cancelTableChoice.data());
    self.m_tableChooser = buildTableChooser(self, MainWindow::Label::chooseTable);
  }
  else {
    setChooseTableButtonLabelToChoose(self);
    self.m_tableChooser = nullptr;
  }
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
  while (Fl::first_window()) { Fl::first_window()->hide(); }
}

struct [[nodiscard]] NbFilesToLoadArgs final {
  Fl_Progress* m_progressBar;
  size_t m_nbFilesToLoad;
};

/**
 * Called by the GUI event loop when the history loader (another thread) notifies
 * of how many file will be loaded.
*/
static inline void setNbFilesToLoadAwakeCb(void* hidden) {
  LOG.debug<__func__>();
  auto args { std::unique_ptr<NbFilesToLoadArgs>(static_cast<NbFilesToLoadArgs*>(hidden)) };
  args->m_progressBar->maximum(static_cast<float>(args->m_nbFilesToLoad));
}

/**
 * Called by the GUI event loop when the history loader (another thread) notifies
 * the loading has made progress.
 */
static inline void incrementProgressBarAwakeCb(void* hidden) {
  LOG.debug<__func__>();
  auto& pb { *static_cast<Fl_Progress*>(hidden) };
  pb.value(pb.value() + 1);
  pb.copy_label(fmt::format("{}/{}", limits::toInt(pb.value()), limits::toInt(pb.maximum())).c_str());
}

/**
 * Called by the GUI event loop when the history loader (another thread) notifies
 * the loading is done.
 */
static inline void finishHistoryLoadingAwakeCb(void* hidden) {
  LOG.debug<__func__>();
  static_cast<Fl_Button*>(hidden)->activate();
}

struct [[nodiscard]] ImportDirArgs final {
  Gui::Implementation& m_self;
  fs::path m_dir;

  ImportDirArgs(Gui::Implementation& self, fs::path dir)
    : m_self(self),
    m_dir(dir) {}
};

/**
 * Called by the GUI event loop when the user chosed a valid history dir.
 * Starts the import process.
 * During the process, the other thread will call
 * - progressBarIncrementAwakeCb() (several times)
 * - setNbFilesToLoadAwakeCb() (once)
 * - historyLoadingDoneAwakeCb() (once)
 * can't use gsl::not_null due to signature being imposed by FLTK.
 */
static inline void importDirAwakeCb(void* hidden) {
  LOG.debug<__func__>();
  auto args { std::unique_ptr<ImportDirArgs>(static_cast<ImportDirArgs*>(hidden)) };
  const auto& historyDir { args->m_dir.filename().string() };
  LOG.info<"The import directory '{}' is valid">(historyDir);
  /* the string is copied in the widget internal buffer*/
  args->m_self.m_histoDirTextField->copy_label(historyDir.c_str());
  args->m_self.m_progressBar->activate();
  LOG.info<"importing history">();
  auto& self { args->m_self };
  args->m_self.m_app.importHistory(args->m_dir,
    [&self]() { Fl::awake(incrementProgressBarAwakeCb, self.m_progressBar); },
    [&self](std::size_t nbFilesToLoad) { Fl::awake(setNbFilesToLoadAwakeCb, new NbFilesToLoadArgs(self.m_progressBar, nbFilesToLoad)); },
    [&self]() { Fl::awake(finishHistoryLoadingAwakeCb, self.m_chooseTableBtn); });
}

/**
* Called by the GUI (button component) when the user clicks on the 'choose
* history directory' button. Displays a directory chooser window.
*/
static inline void choseHistoDirCb(Fl_Widget*, void* hiddenSelf) {
  LOG.debug<__func__>();
  auto& self { *static_cast<Gui::Implementation*>(hiddenSelf) };
  informUser<"">(self);
  auto dirChoser { buildDirectoryChooser(*self.m_preferences) };

  switch (FileChoiceStatus(dirChoser->show())) {
  case FileChoiceStatus::ok: {
    const fs::path dir { dirChoser->filename() };
    LOG.info<"the user chose to import the directory '{}'">(dir.string());

    if (AppInterface::isValidHistory(dir)) {
      saveToPreferences(*self.m_preferences, MainWindow::Label::preferencesKeyChosenDir,
        dir.string().c_str()); // to get const char*
      Fl::awake(importDirAwakeCb, new ImportDirArgs(self, dir));
    }
    else {
      LOG.info<"the chosen directory '{}' is not a valid history dir">(dir.string());
      informUser(self, MainWindow::Label::invalidChoice);
    }
  } break;

  case FileChoiceStatus::error: [[unlikely]] {
    if (nullptr == dirChoser->errmsg()) { informUser<"File choice error">(self); }
    else { informUser(self, dirChoser->errmsg()); }
    } break;

  case FileChoiceStatus::cancel: [[fallthrough]];

  default: /* nothing to do */ break;
  }
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

[[nodiscard]] static inline gsl::not_null<Fl_Menu_Bar*> buildMenuBar(Fl_Preferences* preferences) {
  const auto [x, y, w, h] { MainWindow::Surface::menuBar };
  auto ret { new Fl_Menu_Bar(x, y, w, h) };
  ret->add("&File/E&xit", 0, exitCb, preferences);
  ret->box(FL_NO_BOX);
  return ret;
}

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildChooseHistoDirBtn(
  Gui::Implementation& self) {
  const auto [x, y, w, h] { MainWindow::Surface::chooseHistoDirBtn };
  auto ret { new Fl_Button(x, y, w, h, MainWindow::Label::chooseHistoDir.data()) };
  /* setup the button for choosing which history directory to import into database */
  ret->callback(choseHistoDirCb, &self);
  return ret;
}

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildChooseTableBtn(
  Gui::Implementation& self) {
  const auto [x, y, w, h] { MainWindow::Surface::chooseTableBtn };
  auto ret { new Fl_Button(x, y, w, h, MainWindow::Label::chooseTable.data()) };
  /* setup the table chooser */
  ret->callback(chooseTableCb, &self);
  /* ret->deactivate(); */
  return ret;
}

[[nodiscard]] static inline gsl::not_null<Fl_Button*> buildStopHudBtn(Gui::Implementation& self) {
  const auto [x, y, w, h] { MainWindow::Surface::stopHudBtn };
  auto ret { new Fl_Button(x, y, w, h, MainWindow::Label::stopHud.data()) };
  ret->callback(stopHudCb, &self);
  ret->deactivate();
  return ret;
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildHistoDirTextField(Fl_Preferences& pref) {
  const auto [x, y, w, h] { MainWindow::Surface::histoDirTextField };
  auto ret { new Fl_Box(x, y, w, h) };
  /* setup the text box to display the chosen directory */
  ret->copy_label(phud::filesystem::isDir(getPreferredHistoDir(pref)) ?
    MainWindow::Label::HAND_HISTORY_EXISTS.data() :
    MainWindow::Label::CHOOSE_HAND_HISTORY_DIRECTORY.data());
  return ret;
}

[[nodiscard]] static inline gsl::not_null<Fl_Progress*> buildProgressBar() {
  const auto [x, y, w, h] { MainWindow::Surface::progressBar };
  auto ret { new Fl_Progress(x, y, w, h) };
  ret->color(MainWindow::Color::progressBarBackgroundColor);
  ret->selection_color(MainWindow::Color::progressBarForegroundColor);
  ret->labelcolor(MainWindow::Color::progressBarTextColor);
  ret->minimum(0);
  ret->maximum(1);
  ret->deactivate();
  //ret->box(FL_NO_BOX);
  return ret;
}

[[nodiscard]] static inline gsl::not_null<Fl_Box*> buildInfoBar() {
  const auto [x, y, w, h] { MainWindow::Surface::infoBar };
  return new Fl_Box(x, y, w, h, MainWindow::Label::WELCOME.data());
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

  if (0 == MainWindow::Label::HAND_HISTORY_EXISTS.compare(m_pImpl->m_histoDirTextField->label())) {
    m_pImpl->m_chooseTableBtn->activate();
    m_pImpl->m_app.setHistoryDir(getPreferredHistoDir(*m_pImpl->m_preferences));
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