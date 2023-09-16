#include "containers/algorithms.hpp" // phud::algorithms::*
#include "entities/Hand.hpp"
#include "entities/Player.hpp"
#include "entities/Site.hpp"
#include "filesystem/TextFile.hpp"
#include "history/GameData.hpp" // CashGame, Tournament, Variant, Time
#include "history/WinamaxHandBuilder.hpp"
#include "history/WinamaxGameHistory.hpp"  // Path
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/ProgramInfos.hpp"
#include "threads/PlayerCache.hpp"

#include <optional>

static Logger LOG { CURRENT_FILE_NAME };

namespace pa = phud::algorithms;
namespace ps = phud::strings;

[[nodiscard]] static constexpr Limit fileStemToLimit(StringView fileStem) noexcept {
  if (fileStem.size() < 11) { return Limit::none; }

  if (fileStem.ends_with("_pot-limit")) { return Limit::potLimit; }

  if (fileStem.ends_with("_no-limit")) { return Limit::noLimit; }

  return Limit::none;
}

[[nodiscard]] static constexpr Variant fileStemToVariant(StringView fileStem) noexcept {
  if (ps::contains(fileStem, "_holdem_")) { return Variant::holdem; }

  if (ps::contains(fileStem, "_omaha_")) { return Variant::omaha; }

  if (ps::contains(fileStem, "_omaha5_")) { return Variant::omaha5; }

  return Variant::none;
}

// file: file name, without path and extension
// Fills gameData->m_gameName, gameData->m_isRealMoney, gameData->m_variant and gameData->m_limit
// should parse something like:
// 20141116_Double or Nothing(100679030)_real_holdem_no-limit
// 20150630_Super Freeroll Stade 1 - Déglingos _(123322389)_real_holdem_no-limit
// 20180304_Ferrare 04_real_omaha5_pot-limit
// 20170305_Memphis 06_play_omaha_pot-limit
// "\\d{ 8 }_(.*)_(real|play)?_(.*)_(.*)"
// exported for unit testing
/* static inline */ std::optional<Tuple<bool, String, Variant, Limit>> parseFileStem(
StringView fileStem) {
  LOG.debug<"Parsing the file stem {}.">(fileStem);
  Tuple<bool, String, Variant, Limit> ret {};

  if (12 > fileStem.size()) {
    LOG.error<"Couldn't parse the file stem '{}', too short!!!">(fileStem);
    return ret;
  }

  const auto pos { fileStem.find("_real_", 9) }; // we ignore the date at the start of the file stem
  const auto isRealMoney { StringView::npos != pos };

  if (!isRealMoney and (StringView::npos == fileStem.find("_play_", 9))) [[unlikely]] {
    LOG.error<"Couldn't parse the file stem '{}', unable to guess real or play money!!!">(fileStem);
    return ret;
  }
  const auto gameName { fileStem.substr(9, pos - 9) };
  const auto variant { fileStemToVariant(fileStem) };
  const auto limit { fileStemToLimit(fileStem) };
  ret = { isRealMoney, String(gameName), variant, limit };
  return ret;
}

template <typename GAME_TYPE>
[[nodiscard]] static inline uptr<GAME_TYPE> newGame(StringView gameId, const GameData& gameData) {
  static_assert(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>);

  if constexpr(std::is_same_v<GAME_TYPE, CashGame>) {
    return mkUptr<CashGame>(CashGame::Params {.id = gameId, .siteName = ProgramInfos::WINAMAX_SITE_NAME,
                            .cashGameName = gameData.m_gameName, .variant = gameData.m_variant, .limit = gameData.m_limit,
                            .isRealMoney = gameData.m_isRealMoney, .nbMaxSeats = gameData.m_nbMaxSeats,
                            .smallBlind = gameData.m_smallBlind, .bigBlind = gameData.m_bigBlind, .startDate = gameData.m_startDate});
  }

  if constexpr(std::is_same_v<GAME_TYPE, Tournament>) {
    return mkUptr<Tournament>(Tournament::Params {.id = gameId, .siteName = ProgramInfos::WINAMAX_SITE_NAME,
                              .tournamentName = gameData.m_gameName, .variant = gameData.m_variant, .limit = gameData.m_limit,
                              .isRealMoney = gameData.m_isRealMoney, .nbMaxSeats = gameData.m_nbMaxSeats, .buyIn = gameData.m_buyIn, .startDate = gameData.m_startDate});
  }
}

static inline void fillFromFileName(const Tuple<bool, String, Variant, Limit>& values,
                                    GameData& gameData) {
  const auto [isRealMoney, gameName, variant, limit] { values };
  gameData.m_isRealMoney = isRealMoney;
  gameData.m_gameName = gameName;
  gameData.m_variant = variant;
  gameData.m_limit = limit;
}

template <typename GAME_TYPE> [[nodiscard]] static inline
uptr<GAME_TYPE> createGame(const Path& gameHistoryFile, PlayerCache& cache) {
  LOG.debug<"Creating the game history from {}.">(gameHistoryFile.filename().string());
  const auto& fileStem { ps::sanitize(gameHistoryFile.stem().string()) };
  uptr<GAME_TYPE> ret;

  if (const auto & oGameDataFromFileName { parseFileStem(fileStem) };
      oGameDataFromFileName.has_value()) {
    TextFile tfl { gameHistoryFile };

    while (tfl.next()) {
      if (nullptr == ret) {
        LOG.debug<"1st hand : get additional game data from the hand.">();
        auto [pHand, pGameData] { WinamaxHandBuilder::buildHandAndGameData<GAME_TYPE>(tfl, cache) };
        fillFromFileName(oGameDataFromFileName.value(), *pGameData);
        LOG.debug<"1st hand : creating new game history.">();
        ret = newGame<GAME_TYPE>(fileStem, *pGameData);
        ret->addHand(std::move(pHand));
      } else {
        LOG.debug<"not the 1st hand : adding the new hand to the existing game history.">();
        ret->addHand(WinamaxHandBuilder::buildHand<GAME_TYPE>(tfl, cache));
      }
    }

    LOG.debug<"Read {} line{} from file {}.">(tfl.getLineIndex(), ps::plural(tfl.getLineIndex()),
        tfl.getFileStem());
  }

  return ret;
}

template<typename GAME_TYPE>
[[nodiscard]] static inline uptr<Site> handleGame(const Path& gameHistoryFile) {
  LOG.debug<"Handling the game history from {}.">(gameHistoryFile.filename().string());
  auto pSite { mkUptr<Site>(ProgramInfos::WINAMAX_SITE_NAME) };
  PlayerCache cache { ProgramInfos::WINAMAX_SITE_NAME };

  if (auto g { createGame<GAME_TYPE>(gameHistoryFile, cache) }; nullptr != g) {
    LOG.debug<"Game created for file {}.">(gameHistoryFile.filename().string());
    pSite->addGame(std::move(g));
  } else { LOG.info<"Game *not* created for file {}.">(gameHistoryFile.filename().string()); }

  auto players { cache.extractPlayers() };
  pa::forEach(players, [&](auto & p) { pSite->addPlayer(std::move(p)); });
  return pSite;
}

// reminder: WinamaxGameHistory is a namespace
uptr<Site> WinamaxGameHistory::parseGameHistory(const Path& gameHistoryFile) {
  LOG.debug<"Parsing the {} game history file {}.">(ProgramInfos::WINAMAX_SITE_NAME,
      gameHistoryFile.filename().string());
  const auto& fileStem { gameHistoryFile.stem().string() };

  if (12 > fileStem.size()) {
    LOG.error<"Couldn't parse the file name '{}', too short!!!">(fileStem);
    return mkUptr<Site>(ProgramInfos::WINAMAX_SITE_NAME);
  }

  // history files with an '!' in their title are duplicated with another name, so ignore it
  if (ps::contains(fileStem, '!')) {
    LOG.info<"Ignoring the file '{}' as it start with '!' and thus is duplicated.">(
      gameHistoryFile.filename().string());
    return mkUptr<Site>(ProgramInfos::WINAMAX_SITE_NAME);
  }

  if (String::npos == fileStem.find("_real_", 9) and String::npos == fileStem.find("_play_", 9)) {
    LOG.error<"Couldn't parse the file name '{}', unable to guess real or play money!!!">(fileStem);
    return mkUptr<Site>(ProgramInfos::WINAMAX_SITE_NAME);
  }

  return ps::contains(fileStem, '(') ? handleGame<Tournament>(gameHistoryFile) : handleGame<CashGame>
         (gameHistoryFile);
}
