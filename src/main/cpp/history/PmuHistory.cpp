#include "entities/Game.hpp" // CashGame
#include "entities/Player.hpp" // needed as Site declares an incomplete Player type
#include "entities/Site.hpp"        // Site
#include "history/PmuHistory.hpp" // std::filesystem::path

namespace fs = std::filesystem;
namespace pf = phud::filesystem;

PmuHistory::~PmuHistory() = default;

/**
 * @return true if the given dir is a direct child of a HandHistory dir, and contains at least one subdir, named YYYYMMDD.
 */
bool PmuHistory::isValidHistory(const fs::path& /*historyDir*/) {
  return false;
}

std::unique_ptr<Site> PmuHistory::load(const fs::path& /*historyDir*/, FunctionVoid /*incrementCb*/,
                            FunctionInt /*setNbFilesCb*/) {
  return nullptr;
}

/* [[nodiscard]] static*/ std::unique_ptr<Site> PmuHistory::load(const fs::path& /*historyDir*/) { return nullptr; }

void PmuHistory::stopLoading() {}

std::unique_ptr<Site> PmuHistory::reloadFile(const fs::path& /*winamaxHistoryFile*/) {
  return nullptr;
}

std::string_view PmuHistory::getTableNameFromTableWindowTitle(std::string_view /*tableWindowTitle*/) const {
  return "";
}

fs::path PmuHistory::getHistoryFileFromTableWindowTitle(const fs::path& /*historyDir*/,
    std::string_view /*tableWindowTitle*/) const {
  return {};
}