#include "entities/Player.hpp"    // needed as Site declares an incomplete Player type
#include "entities/Site.hpp"      // Site
#include "history/PmuHistory.hpp" // std::filesystem::path

namespace fs = std::filesystem;

PmuHistory::~PmuHistory() = default;

/**
 * @return true if the given dir is a direct child of a HandHistory dir, and contains at least one
 * subdir, named YYYYMMDD.
 */
bool PmuHistory::isValidHistory(const fs::path& /*historyDir*/) noexcept {
  return false;
}

std::unique_ptr<Site> PmuHistory::load(const fs::path& /*historyDir*/,
                                       std::function<void()> /*onProgress*/,
                                       std::function<void(std::size_t)> /*onSetNbFiles*/) noexcept {
  return nullptr;
}

/* [[nodiscard]] static*/ std::unique_ptr<Site> PmuHistory::load(const fs::path& /*historyDir*/) noexcept {
  return nullptr;
}

void PmuHistory::stopLoading() noexcept {}

std::unique_ptr<Site> PmuHistory::reloadFile(const fs::path& /*winamaxHistoryFile*/) noexcept {
  return nullptr;
}

std::string_view
PmuHistory::getTableNameFromTableWindowTitle(std::string_view /*tableWindowTitle*/) const noexcept {
  return "";
}

std::optional<fs::path>
PmuHistory::getHistoryFileFromTableWindowTitle(const fs::path& /*historyDir*/,
                                               std::string_view /*tableWindowTitle*/) const noexcept {
  return {};
}
