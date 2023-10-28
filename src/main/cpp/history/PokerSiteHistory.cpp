#include "entities/Game.hpp" // // needed as Site declares incomplete CashGame type
#include "entities/Player.hpp" // needed as Site declares incomplete Player type
#include "entities/Site.hpp"
#include "history/PmuHistory.hpp" // PokerSiteHistory
#include "history/WinamaxHistory.hpp"

namespace pf = phud::filesystem;

PokerSiteHistory::~PokerSiteHistory() = default;

/* [[nodiscard]] static */ uptr<PokerSiteHistory> PokerSiteHistory::newInstance(
  const pf::Path& historyDir) {
  if (PmuHistory::isValidHistory(historyDir)) { return mkUptr<PmuHistory>(); }

  if (WinamaxHistory::isValidHistory(historyDir)) { return mkUptr<WinamaxHistory>(); }

  return nullptr;
}

/* [[nodiscard]] static */ bool PokerSiteHistory::isValidHistory(const pf::Path& historyDir) {
  return PmuHistory::isValidHistory(historyDir) or WinamaxHistory::isValidHistory(historyDir);
}

/* [[nodiscard]] static */ uptr<Site> PokerSiteHistory::load(const pf::Path& historyDir) {
  if (PmuHistory::isValidHistory(historyDir)) { return PmuHistory::load(historyDir); }

  if (WinamaxHistory::isValidHistory(historyDir)) { return WinamaxHistory::load(historyDir); }

  return nullptr;
}