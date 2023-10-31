#include "entities/Game.hpp" // // needed as Site declares incomplete CashGame type
#include "entities/Player.hpp" // needed as Site declares incomplete Player type
#include "entities/Site.hpp"
#include "history/PmuHistory.hpp" // PokerSiteHistory
#include "history/WinamaxHistory.hpp"

namespace fs = std::filesystem;

PokerSiteHistory::~PokerSiteHistory() = default;

/* [[nodiscard]] static */ std::unique_ptr<PokerSiteHistory> PokerSiteHistory::newInstance(
  const fs::path& historyDir) {
  if (PmuHistory::isValidHistory(historyDir)) { return std::make_unique<PmuHistory>(); }

  if (WinamaxHistory::isValidHistory(historyDir)) { return std::make_unique<WinamaxHistory>(); }

  return nullptr;
}

/* [[nodiscard]] static */ bool PokerSiteHistory::isValidHistory(const fs::path& historyDir) {
  return PmuHistory::isValidHistory(historyDir) or WinamaxHistory::isValidHistory(historyDir);
}

/* [[nodiscard]] static */ std::unique_ptr<Site> PokerSiteHistory::load(
  const fs::path& historyDir) {
  if (PmuHistory::isValidHistory(historyDir)) { return PmuHistory::load(historyDir); }

  if (WinamaxHistory::isValidHistory(historyDir)) { return WinamaxHistory::load(historyDir); }

  return nullptr;
}