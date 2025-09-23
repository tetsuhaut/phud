#include "gui/TableService.hpp"
#include "gui/WindowUtils.hpp"
#include "language/limits.hpp"
#include "language/Validator.hpp"
#include "log/Logger.hpp"
#include <psapi.h> // GetModuleFileNameEx
#include <gsl/gsl>

static Logger LOG { CURRENT_FILE_NAME };

std::string getLastErrorMessageFromOS() {
  const auto localeId { LocaleNameToLCID(LOCALE_NAME_SYSTEM_DEFAULT, 0) };
  char err[MAX_PATH + 1] { '\0' };
  const auto size { FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), localeId, &err[0], MAX_PATH, nullptr) };
  err[MAX_PATH] = '\0';
  return 0 == size ? "Failed to retrieve error message from system" : std::string(&err[0]);
}

std::string getExecutableName(const HWND window) {
  LOG.debug<__func__>();

  if (nullptr == window) { return ""; }

  DWORD pid;
  GetWindowThreadProcessId(window, &pid);
  const auto myProcessHandle { OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid) };
  validation::requireNotNull(myProcessHandle, getLastErrorMessageFromOS().c_str());
  const auto _ { gsl::finally([myProcessHandle] { CloseHandle(myProcessHandle); }) };
  char process[MAX_PATH + 1] { '\0' };

  if (const auto nbChars { GetModuleFileNameEx(myProcessHandle, nullptr, &process[0], MAX_PATH) }; 0 != nbChars) {
    process[MAX_PATH] = '\0';
    return std::string(&process[0]);
  }
  LOG.error<"Can't retrieve the executable name: {}">(getLastErrorMessageFromOS());
  return "";
}

ErrorOrRectangleAndName getWindowRectangleAndName(TableService& tableService, int x, int y) {
  LOG.debug<__func__>();
  const auto& myWindowHandle { WindowFromPoint({x, y}) };

  if (nullptr == myWindowHandle) { 
    return ErrorOrRectangleAndName::err<"No window at the given position">(); 
  }

  if (!tableService.isPokerApp(getExecutableName(myWindowHandle))) { 
    return ErrorOrRectangleAndName::err<"The chosen window is not a poker table.">(); 
  }

  if (RECT r; 0 != GetWindowRect(myWindowHandle, &r)) {
    char tableName[MAX_PATH + 1] { '\0' };
    GetWindowText(myWindowHandle, &tableName[0], MAX_PATH);
    tableName[MAX_PATH] = '\0';
    return ErrorOrRectangleAndName::res({ toRectangle(r), tableName });
  }

  return ErrorOrRectangleAndName::err<"Could not get the chosen window handle.">();
}

std::vector<std::string> getWindowTitles() {
  std::vector<std::string> titles;

  EnumWindows([](HWND hwnd, LPARAM hiddenTitles) -> BOOL {
    auto& localTitles { *reinterpret_cast<std::vector<std::string>*>(hiddenTitles) };

    if (IsWindowVisible(hwnd)) {
      // Get window title
      const auto length { GetWindowTextLength(hwnd) };
      const auto lengthPlusOne { length + 1 };
      auto title { std::string(limits::toSizeT(lengthPlusOne), '\0') };

      if (const auto actualLength { GetWindowText(hwnd, title.data(), lengthPlusOne) }; actualLength > 0) {
        title.resize(limits::toSizeT(actualLength));
        localTitles.emplace_back(title);
      }
    }

    return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&titles));

  return titles;
}

std::optional<phud::Rectangle> getTableWindowRectangle(std::string_view tableName) {
  // Find window position by title
  if (const auto& hwnd { FindWindow(nullptr, tableName.data()) }; nullptr != hwnd) {
    if (RECT rect; 0 != GetWindowRect(hwnd, &rect)) {
      phud::Rectangle tableRect = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
      return tableRect;
    }
  }
  return {};
}
