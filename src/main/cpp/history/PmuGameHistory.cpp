#include "constants/ProgramInfos.hpp" // ProgramInfos::*
#include "entities/Hand.hpp"
#include "entities/Player.hpp"
#include "entities/Site.hpp"
#include "filesystem/TextFile.hpp"
#include "history/GameData.hpp" // CashGame, Tournament, Variant, Time
#include "history/PmuHandBuilder.hpp"
#include "history/PmuGameHistory.hpp" // std::filesystem::path
#include "log/Logger.hpp"             // CURRENT_FILE_NAME
#include "strings/StringUtils.hpp"    // phud::strings::*
#include "threads/PlayerCache.hpp"

static Logger& LOG() {
  static Logger logger {CURRENT_FILE_NAME};
  return logger;
}

namespace fs = std::filesystem;
namespace ps = phud::strings;


template <typename GAME_TYPE>
  requires(std::is_same_v<GAME_TYPE, CashGame> or std::is_same_v<GAME_TYPE, Tournament>)
[[nodiscard]] static std::unique_ptr<GAME_TYPE> newGame(std::string_view gameId,
                                                        const GameData& gameData) {
  if constexpr (std::is_same_v<GAME_TYPE, CashGame>) {
    return std::make_unique<CashGame>(CashGame::Params {.id = gameId,
                                                        .siteName = ProgramInfos::PMU_SITE_NAME,
                                                        .cashGameName = gameData.m_gameName,
                                                        .variant = gameData.m_variant,
                                                        .limit = gameData.m_limit,
                                                        .isRealMoney = gameData.m_isRealMoney,
                                                        .nbMaxSeats = gameData.m_nbMaxSeats,
                                                        .smallBlind = gameData.m_smallBlind,
                                                        .bigBlind = gameData.m_bigBlind,
                                                        .startDate = gameData.m_startDate});
  }

  if constexpr (std::is_same_v<GAME_TYPE, Tournament>) {
    return std::make_unique<Tournament>(Tournament::Params {.id = gameId,
                                                            .siteName = ProgramInfos::PMU_SITE_NAME,
                                                            .tournamentName = gameData.m_gameName,
                                                            .variant = gameData.m_variant,
                                                            .limit = gameData.m_limit,
                                                            .isRealMoney = gameData.m_isRealMoney,
                                                            .nbMaxSeats = gameData.m_nbMaxSeats,
                                                            .buyIn = gameData.m_buyIn,
                                                            .startDate = gameData.m_startDate});
  }
}

template <typename GAME_TYPE>
[[nodiscard]] static std::unique_ptr<GAME_TYPE> createGame(const fs::path& gameHistoryFile,
                                                           PlayerCache& cache) {
  LOG().debug<"Creating the game history from {}.">(gameHistoryFile.filename().string());
  const auto fileStem = ps::sanitize(gameHistoryFile.stem().string());
  std::unique_ptr<GAME_TYPE> ret;
  TextFile tf(gameHistoryFile);

  while (tf.next()) {
    if (nullptr == ret) {
      LOG().debug<"1st hand : get additional game data from the hand.">();
      auto [pHand, pGameData] {PmuHandBuilder::buildHandAndGameData<GAME_TYPE>(tf, cache)};
      LOG().debug<"1st hand : creating new game history.">();
      ret = newGame<GAME_TYPE>(fileStem, *pGameData);
      ret->addHand(std::move(pHand));
      // no way to know when we finished reading winners -> pass to next hand directly
    } else {
      LOG().debug<"not the 1st hand : adding the new hand to the existing game history.">();
      ret->addHand(PmuHandBuilder::buildHand<GAME_TYPE>(tf, cache));
    }
  }

  LOG().debug<"Read {} line{} from file {}.">(tf.getLineIndex(), ps::plural(tf.getLineIndex()),
                                              tf.getFileStem());
  return ret;
}

template <typename GAME_TYPE>
[[nodiscard]] static std::unique_ptr<GAME_TYPE>
createGame(auto,
           PlayerCache&) = delete; // use only std::filesystem::path

template <typename GAME_TYPE>
[[nodiscard]] static std::unique_ptr<Site> handleGame(const fs::path& gameHistoryFile) {
  LOG().debug<"Handling the game history from {}.">(gameHistoryFile.filename().string());
  auto pSite = std::make_unique<Site>(ProgramInfos::PMU_SITE_NAME);
  PlayerCache cache {ProgramInfos::PMU_SITE_NAME};

  if (auto g {createGame<GAME_TYPE>(gameHistoryFile, cache)}; nullptr != g) {
    LOG().debug<"Game created for file {}.">(gameHistoryFile.filename().string());
    pSite->addGame(std::move(g));
  } else {
    LOG().info<"Game *not* created for file {}.">(gameHistoryFile.filename().string());
  }

  auto players = cache.extractPlayers();
  std::ranges::for_each(players, [&](auto& p) { pSite->addPlayer(std::move(p)); });
  return pSite;
}

std::unique_ptr<Site> PmuGameHistory::parseGameHistory(const fs::path& gameHistoryFile) {
  LOG().debug<"Parsing the {} game history file {}.">(ProgramInfos::PMU_SITE_NAME,
                                                      gameHistoryFile.filename().string());
  return handleGame<CashGame>(gameHistoryFile);
}
