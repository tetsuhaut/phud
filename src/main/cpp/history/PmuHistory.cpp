#include "entities/Game.hpp" // CashGame
#include "entities/Player.hpp" // needed as Site declares incomplete Player type
#include "entities/Site.hpp"        // Site
#include "history/PmuHistory.hpp" // Path

PmuHistory::~PmuHistory() = default;

/**
 * @return true if the given dir is a direct child of a HandHistory dir, and contains at least one subdir, named YYYYMMDD.
 */
bool PmuHistory::isValidHistory(const Path& /*historyDir*/) {
  return false;
}

uptr<Site> PmuHistory::load(const Path& /*historyDir*/, FunctionVoid /*incrementCb*/,
                            FunctionInt /*setNbFilesCb*/) {
  return nullptr;
}

/* [[nodiscard]] static*/ uptr<Site> PmuHistory::load(const Path& /*historyDir*/) { return nullptr; }

void PmuHistory::stopLoading() {}

uptr<Site> PmuHistory::reloadFile(const Path& /*winamaxHistoryFile*/) {
  return nullptr;
}

StringView PmuHistory::getTableNameFromTableWindowTitle(StringView /*tableWindowTitle*/) const {
  return "";
}

Path PmuHistory::getHistoryFileFromTableWindowTitle(const Path& /*historyDir*/,
    StringView /*tableWindowTitle*/) const {
  return {};
}