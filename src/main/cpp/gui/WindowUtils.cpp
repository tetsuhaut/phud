#include "gui/TableService.hpp"
#include "gui/WindowUtils.hpp" // namespace mswindows
#include "language/limits.hpp"
#include "language/Validator.hpp"
#include "log/Logger.hpp"
#include <Psapi.h> // GetModuleFileNameEx
#include <gsl/gsl>

//static Logger& LOG() {
//  static Logger logger { CURRENT_FILE_NAME };
//  return logger;
//}

namespace {
  /**
   * Gets the last error message from the OS.
   * @return Formatted error message
   */
  // std::string getLastErrorMessageFromOS() {
  //   const auto localeId { LocaleNameToLCID(LOCALE_NAME_SYSTEM_DEFAULT, 0) };
  //   char err[MAX_PATH + 1] {};
  //   const auto size {
  //     FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), localeId, &err[0], MAX_PATH, nullptr)
  //   };
  //   err[MAX_PATH] = '\0';
  //   return 0 == size ? "Failed to retrieve error message from system" : std::string(&err[0]);
  // }

  /**
 * Gets the executable name of a window process.
 * @param window Window handle
 * @return Executable name or empty string on error
 */
  // std::string getExecutableName(HWND window) {
  //   LOG().debug<__func__>();
  //
  //   if (nullptr == window) { return ""; }
  //
  //   DWORD pid;
  //   GetWindowThreadProcessId(window, &pid);
  //   const auto myProcessHandle { OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid) };
  //   validation::requireNotNull(myProcessHandle, getLastErrorMessageFromOS());
  //   const auto _ { gsl::finally([myProcessHandle] { CloseHandle(myProcessHandle); }) };
  //   char process[MAX_PATH + 1] {};
  //
  //   if (const auto nbChars { GetModuleFileNameEx(myProcessHandle, nullptr, &process[0], MAX_PATH) }; 0 != nbChars) {
  //     process[MAX_PATH] = '\0';
  //     return &process[0];
  //   }
  //   LOG().error<"Can't retrieve the executable name: {}">(getLastErrorMessageFromOS());
  //   return "";
  // }

  /**
 * Converts a RECT object from the Windows API into a phud::Rectangle object.
 * @param r Windows RECT structure
 * @return phud::Rectangle equivalent
 */
  [[nodiscard]] constexpr phud::Rectangle toRectangle(const RECT& r) noexcept {
    return { r.left, r.top, r.right - r.left, r.bottom - r.top };
  }

  BOOL windowTitleCallback(HWND hwnd, LPARAM hiddenTitles) {
    auto& localTitles = *reinterpret_cast<std::vector<std::string>*>(hiddenTitles);

    if (IsWindowVisible(hwnd)) {
      // Get window title
      const auto length = GetWindowTextLength(hwnd);
      const auto lengthPlusOne = length + 1;
      auto title = std::string(limits::toSizeT(lengthPlusOne), '\0');

      if (const auto actualLength { GetWindowText(hwnd, title.data(), lengthPlusOne) }; actualLength > 0) {
        title.resize(limits::toSizeT(actualLength));
        localTitles.emplace_back(title);
      }
    }

    return TRUE; // Continue enumeration
  }
} // anonymous namespace

std::vector<std::string> mswindows::getWindowTitles() {
  std::vector<std::string> titles;
  EnumWindows(windowTitleCallback, reinterpret_cast<LPARAM>(&titles));
  return titles;
}

std::optional<phud::Rectangle> mswindows::getTableWindowRectangle(std::string_view tableWindowTitle) {
  // Find window position by title
  if (const auto hwnd = FindWindow(nullptr, tableWindowTitle.data()); nullptr != hwnd) {
    if (RECT rect; 0 != GetWindowRect(hwnd, &rect)) {
      return toRectangle(rect);
    }
  }
  return {};
}

void mswindows::setWindowOnTopMost(HWND above) noexcept {
  /* from https://www.fltk.org/newsgroups.php?s39452+gfltk.general+v39464 */
  SetWindowPos(above, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
}
