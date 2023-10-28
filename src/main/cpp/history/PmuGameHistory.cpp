#include "containers/algorithms.hpp" // phud::algorithms::*
#include "entities/Hand.hpp"
#include "entities/Player.hpp"
#include "entities/Site.hpp"
#include "filesystem/TextFile.hpp"
#include "history/GameData.hpp" // CashGame, Tournament, Variant, Time
#include "history/PmuHandBuilder.hpp"
#include "history/PmuGameHistory.hpp" // Path
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/ProgramInfos.hpp" // ProgramInfos::*
#include "threads/PlayerCache.hpp"

static Logger LOG { CURRENT_FILE_NAME };

namespace pa = phud::algorithms;
namespace ps = phud::strings;

template <typename GAME_TYPE>
[[nodiscard]] static inline uptr<GAME_TYPE> newGame(std::string_view gameId, const GameData& gameData) {
  static_assert(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>);

  if constexpr(std::is_same_v<GAME_TYPE, CashGame>) {
    return mkUptr<CashGame>(CashGame::Params { .id = gameId, .siteName = ProgramInfos::PMU_SITE_NAME,
                            .cashGameName = gameData.m_gameName, .variant = gameData.m_variant, .limit = gameData.m_limit,
                            .isRealMoney = gameData.m_isRealMoney, .nbMaxSeats = gameData.m_nbMaxSeats,
                            .smallBlind = gameData.m_smallBlind, .bigBlind = gameData.m_bigBlind, .startDate = gameData.m_startDate });
  }

  if constexpr(std::is_same_v<GAME_TYPE, Tournament>) {
    return mkUptr<Tournament>(Tournament::Params { .id = gameId, .siteName = ProgramInfos::PMU_SITE_NAME,
                              .tournamentName = gameData.m_gameName, .variant = gameData.m_variant, .limit = gameData.m_limit,
                              .isRealMoney = gameData.m_isRealMoney, .nbMaxSeats = gameData.m_nbMaxSeats, .buyIn = gameData.m_buyIn, .startDate = gameData.m_startDate });
  }
}

template <typename GAME_TYPE> [[nodiscard]] static inline
uptr<GAME_TYPE> createGame(const Path& gameHistoryFile, PlayerCache& cache) {
  LOG.debug<"Creating the game history from {}.">(gameHistoryFile.filename().string());
  const auto& fileStem { ps::sanitize(gameHistoryFile.stem().string()) };
  uptr<GAME_TYPE> ret;
  TextFile tf { gameHistoryFile };

  while (tf.next()) {
    if (nullptr == ret) {
      LOG.debug<"1st hand : get additional game data from the hand.">();
      auto [pHand, pGameData] { PmuHandBuilder::buildHandAndGameData<GAME_TYPE>(tf, cache) };
      LOG.debug<"1st hand : creating new game history.">();
      ret = newGame<GAME_TYPE>(fileStem, *pGameData);
      ret->addHand(std::move(pHand));
      // no way to know when we finished reading winners -> pass to next hand directly
    } else {
      LOG.debug<"not the 1st hand : adding the new hand to the existing game history.">();
      ret->addHand(PmuHandBuilder::buildHand<GAME_TYPE>(tf, cache));
    }
  }

  LOG.debug<"Read {} line{} from file {}.">(tf.getLineIndex(), ps::plural(tf.getLineIndex()),
      tf.getFileStem());
  return ret;
}

template <typename GAME_TYPE> [[nodiscard]] static inline
uptr<GAME_TYPE> createGame(auto, PlayerCache&) = delete; // use only Path

template<typename GAME_TYPE>
[[nodiscard]] static inline uptr<Site> handleGame(const Path& gameHistoryFile) {
  LOG.debug<"Handling the game history from {}.">(gameHistoryFile.filename().string());
  auto pSite { mkUptr<Site>(ProgramInfos::PMU_SITE_NAME) };
  PlayerCache cache { ProgramInfos::PMU_SITE_NAME };

  if (auto g { createGame<GAME_TYPE>(gameHistoryFile, cache) }; nullptr != g) {
    LOG.debug<"Game created for file {}.">(gameHistoryFile.filename().string());
    pSite->addGame(std::move(g));
  } else { LOG.info<"Game *not* created for file {}.">(gameHistoryFile.filename().string()); }

  auto players { cache.extractPlayers() };
  pa::forEach(players, [&](auto & p) { pSite->addPlayer(std::move(p)); });
  return pSite;
}

uptr<Site> PmuGameHistory::parseGameHistory(const Path& gameHistoryFile) {
  LOG.debug<"Parsing the {} game history file {}.">(ProgramInfos::PMU_SITE_NAME,
      gameHistoryFile.filename().string());
  return handleGame<CashGame>(gameHistoryFile);
}