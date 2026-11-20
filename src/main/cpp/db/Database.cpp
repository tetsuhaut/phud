#include "constants/TableConstants.hpp"
#include "db/Database.hpp" // DatabaseException, std::string, std::vector, std::ostream (specialized by fmt to use operator<< with Database), std::span
#include "db/PreparedStatement.hpp"
#include "db/sqlQueries.hpp" // all the SQL queries
#include "entities/Action.hpp"
#include "entities/Card.hpp" // toString(Card)
#include "entities/Game.hpp" // Cashgame, Limit, Time, Tournament, Variant
#include "entities/GameType.hpp"
#include "entities/Hand.hpp" // std::array
#include "entities/Seat.hpp"
#include "entities/Site.hpp"        // Site
#include "filesystem/FileUtils.hpp" // phud::filesystem
#include "language/Validator.hpp"
#include "log/Logger.hpp" // fmt::format(), CURRENT_FILE_NAME
#include "statistics/HandPlayerStats.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "strings/StringUtils.hpp" // phud::strings::contains
#include "threads/ThreadPool.hpp"        // ThreadPool, Future
#include <gsl/gsl>                       // gsl::not_null
#include <sqlite3.h>                     // sqlite3*
#include <stlab/concurrency/utility.hpp> // stlab::await
#include <mutex>
#include <ranges>
#include <utility> // std::forward

// from sqlite3.h: 'The application does not need to worry about freeing the result.' So no need to
// free the char* returned by sqlite3_errmsg().

// FUTURE: the read query name, content, entity type, and callback should be linked.
// FUTURE: use a cache

static constexpr Logger LOG {CURRENT_FILE_NAME};

namespace fs = std::filesystem;
namespace pf = phud::filesystem;
namespace ps = phud::strings;

namespace {
  constexpr std::string_view IN_MEMORY {":memory:"};

  /**
   * Opens the database, this will create the database file if needed.
   * @throws DatabaseException in case of error
   */
  gsl::not_null<sqlite3*> openDatabase(std::string_view dbName) {
    sqlite3* pDb {nullptr};

    if (SQLITE_OK != sqlite3_open(dbName.data(), &pDb)) {
      // sqlite3_errmsg() and sqlite3_close() are null safe
      const auto msg =
          (nullptr != pDb) ? sqlite3_errmsg(pDb) : "Failed to allocate database handle";
      sqlite3_close(pDb); // try to free pDb
      throw DatabaseException(fmt::format("Can't open database file '{}': {}", dbName, msg));
    }

    return pDb;
  }

  /**
   * Runs a query against the database.
   * @throws DatabaseException if an error occurs
   */
  void executeSql(const gsl::not_null<sqlite3*> pDb, std::string_view sql) {
    LOG.trace(sql);

    if (SQLITE_OK != sqlite3_exec(pDb, sql.data(), /*callback*/ nullptr, /*callbackParam*/ nullptr,
                                  /*errorMsg*/ nullptr)) {
      throw DatabaseException(fmt::format("Can't execute the query\n{}\nerror message:\n{}\n", sql,
                                          sqlite3_errmsg(pDb)));
    }
  }

  /**
   * @throws DatabaseException in case of problem getting the query SQL code or opening the database
   * file
   */
  [[nodiscard]] gsl::not_null<sqlite3*> createDatabase(std::string_view name) {
    validation::requireNonEmpty(name, "db name");
    const auto dbFile = fs::path(name);
    const auto isDbCreation = !pf::isFile(dbFile);

    if (isDbCreation) {
      LOG.info<"Creating the database {}">(IN_MEMORY == name ? "in memory"
                                                             : pf::absolute(dbFile).string());
    } else {
      LOG.info<"Opening the existing database file {}, no schema created.">(
          pf::absolute(dbFile).string());
    }

    const auto pDb = openDatabase(name); // will create the database file if needed

    if (IN_MEMORY != name) { // use Write-Ahead Logging for better performance
      executeSql(pDb, "PRAGMA journal_mode=WAL;"); 
      executeSql(pDb, "PRAGMA synchronous=NORMAL;");
    }

    if (isDbCreation) {
      LOG.info<"creating the database schema {}">(name);
      std::ranges::for_each(phud::sql::CREATE_QUERIES,
                            [pDb](const auto& query) { executeSql(pDb, query); });
      LOG.info<"database created">();
    }
    
    return pDb;
  }

  /**
   * Every insertion of the database, prepared once for the database connection.
   */
  struct [[nodiscard]] PreparedInserts final {
    PreparedStatement site;
    PreparedStatement player;
    PreparedStatement game;
    PreparedStatement tournament;
    PreparedStatement cashGame;
    PreparedStatement hand;
    PreparedStatement handPlayer;
    PreparedStatement handPlayerStats;
    PreparedStatement cashGameHand;
    PreparedStatement tournamentHand;
    PreparedStatement action;

    explicit PreparedInserts(gsl::not_null<sqlite3*> db)
      : site {db, phud::sql::INSERT_SITE},
        player {db, phud::sql::INSERT_PLAYER},
        game {db, phud::sql::INSERT_GAME},
        tournament {db, phud::sql::INSERT_TOURNAMENT},
        cashGame {db, phud::sql::INSERT_CASHGAME},
        hand {db, phud::sql::INSERT_HAND},
        handPlayer {db, phud::sql::INSERT_HAND_PLAYER},
        handPlayerStats {db, phud::sql::INSERT_HAND_PLAYER_STATS},
        cashGameHand {db, phud::sql::INSERT_CASHGAME_HAND},
        tournamentHand {db, phud::sql::INSERT_TOURNAMENT_HAND},
        action {db, phud::sql::INSERT_ACTION} {}

    /**
     * @returns the insertion linking a hand to its game
     */
    [[nodiscard]] PreparedStatement& gameHand(GameType gameType) noexcept {
      return (GameType::tournament == gameType) ? tournamentHand : cashGameHand;
    }
  }; // struct PreparedInserts

  void insertGame(PreparedStatement& insert, const auto& game) {
    const auto startDate = game.getStartDate().toSqliteDate(); // must live until execute()
    insert
        .text(":gameId", game.getId()) // use the tournament ID for the Game
        .text(":siteName", game.getSiteName())
        .text(":gameName", game.getName())
        .text(":variant", toString(game.getVariant()))
        .text(":limitType", toString(game.getLimitType()))
        .boolean(":isRealMoney", game.isRealMoney())
        .text(":nbMaxSeats", tableSeat::toString(game.getMaxNbSeats()))
        .text(":startDate", startDate)
        .execute();
  }

  void insertSpecificGame(PreparedInserts& inserts, const Tournament& tournament) {
    LOG.debug<"saving the tournament with id={}">(tournament.getId());
    inserts.tournament.text(":tournamentId", tournament.getId())
        .real(":buyIn", tournament.getBuyIn())
        .execute();
  }

  void insertSpecificGame(PreparedInserts& inserts, const CashGame& cashGame) {
    LOG.debug<"saving the cash game with id={}">(cashGame.getId());
    inserts.cashGame.text(":cashGameId", cashGame.getId())
        .real(":smallBlind", cashGame.getSmallBlind())
        .real(":bigBlind", cashGame.getBigBlind())
        .execute();
  }

  void insertHand(PreparedStatement& insert, const Hand& hand) {
    const auto startDate = hand.getStartDate().toSqliteDate(); // must live until execute()
    insert.text(":handId", hand.getId())
        .text(":siteName", hand.getSiteName())
        .text(":tableName", hand.getTableName())
        .text(":buttonSeat", tableSeat::toString(hand.getButtonSeat()))
        .text(":maxSeats", tableSeat::toString(hand.getMaxSeats()))
        .integer(":ante", hand.getAnte())
        .integer(":level", hand.getLevel())
        .text(":startDate", startDate)
        .text(":heroCard1", toString(hand.getHeroCard1()))
        .text(":heroCard2", toString(hand.getHeroCard2()))
        .text(":heroCard3", toString(hand.getHeroCard3()))
        .text(":heroCard4", toString(hand.getHeroCard4()))
        .text(":heroCard5", toString(hand.getHeroCard5()))
        .text(":boardCard1", toString(hand.getBoardCard1()))
        .text(":boardCard2", toString(hand.getBoardCard2()))
        .text(":boardCard3", toString(hand.getBoardCard3()))
        .text(":boardCard4", toString(hand.getBoardCard4()))
        .text(":boardCard5", toString(hand.getBoardCard5()))
        .execute();
  }

  void insertHandPlayers(PreparedStatement& insert, const Hand& hand) {
    const auto seats = hand.getSeats();
    validation::require(!std::ranges::all_of(seats, [](const auto& p) { return p.empty(); }),
                        "trying to save a hand with no players");
    const auto isSeated = [](const auto& indexedPlayer) {
      return !std::get<1>(indexedPlayer).empty();
    };

    // we suppose the Player has been saved before this call
    for (const auto& [index, playerName] :
         seats | std::views::enumerate | std::views::filter(isSeated)) {
      insert.text(":handId", hand.getId())
          .text(":playerName", playerName)
          .text(":playerSeat", tableSeat::toString(tableSeat::fromArrayIndex(index)))
          .boolean(":isWinner", hand.isWinner(playerName))
          .execute();
    }
  }

  void insertHandPlayerStats(PreparedStatement& insert, const Hand& hand,
                             const HandPlayerStats& stats) {
    insert.text(":handId", hand.getId())
        .text(":playerName", stats.playerName)
        .text(":siteName", hand.getSiteName())
        .text(":gameType", toString(hand.getGameType()))
        .boolean(":vpip", stats.vpip)
        .boolean(":pfr", stats.pfr)
        .boolean(":limp", stats.limp)
        .boolean(":facedRaise", stats.facedRaise)
        .boolean(":foldToRaise", stats.foldToRaise)
        .boolean(":callRaise", stats.callRaise)
        .boolean(":threeBet", stats.threeBet)
        .boolean(":couldSqueeze", stats.couldSqueeze)
        .boolean(":squeeze", stats.squeeze)
        .boolean(":facedThreeBet", stats.facedThreeBet)
        .boolean(":foldToThreeBet", stats.foldToThreeBet)
        .boolean(":callThreeBet", stats.callThreeBet)
        .boolean(":fourBet", stats.fourBet)
        .execute();
  }

  void insertAction(PreparedStatement& insert, const Hand& hand, const Action& action) {
    insert.text(":street", toString(action.getStreet()))
        .text(":handId", hand.getId())
        .text(":playerName", action.getPlayerName())
        .text(":actionType", toString(action.getType()))
        .integer(":actionIndex", gsl::narrow<long long>(action.getIndex()))
        .real(":amount", action.getAmount())
        .execute();
  }

  /**
   * Saves a hand, its players, their statistics and actions, and links it to its game.
   * @throws DatabaseException if an error occurs during the insert
   */
  void saveHand(PreparedInserts& inserts, std::string_view gameId, const Hand& hand) {
    insertHand(inserts.hand, hand);
    insertHandPlayers(inserts.handPlayer, hand);
    std::ranges::for_each(computeHandPlayerStats(hand), [&inserts, &hand](const auto& stats) {
      insertHandPlayerStats(inserts.handPlayerStats, hand, stats);
    });
    inserts.gameHand(hand.getGameType())
        .text(":gameId", gameId)
        .text(":handId", hand.getId())
        .execute();
    LOG.trace<"saving {} actions from hand with id={}">(hand.viewActions().size(), hand.getId());
    std::ranges::for_each(hand.viewActions(), [&inserts, &hand](const auto* pAction) {
      insertAction(inserts.action, hand, *pAction);
    });
  }

  /**
   * @throws DatabaseException if an error occurs during the insert
   */
  void saveGame(PreparedInserts& inserts, const auto& game) {
    const auto hands = game.viewHands();
    const auto gameId = game.getId();
    insertGame(inserts.game, game);
    insertSpecificGame(inserts, game);
    LOG.debug<"saving {} hands from the game with id={}">(hands.size(), gameId);
    std::ranges::for_each(
        hands, [&inserts, &gameId](const auto* pHand) { saveHand(inserts, gameId, *pHand); });
  }
} // anonymous namespace

struct [[nodiscard]] Database::Implementation final {
  std::string m_dbName;
  gsl::not_null<sqlite3*> m_database;
  // the history import and the history watching can save at the same time
  std::mutex m_saveMutex {};
  std::unique_ptr<PreparedInserts> m_inserts;

  explicit Implementation(std::string_view dbName)
    : m_dbName {dbName},
      m_database {createDatabase(dbName)},
      m_inserts {std::make_unique<PreparedInserts>(m_database)} {}
}; // struct  Database::Implementation

class [[nodiscard]] Transaction final {
private:
  std::mutex m_mutex {};
  gsl::not_null<sqlite3*> m_db;
  bool m_didCommit = false;

public:
  explicit Transaction(gsl::not_null<sqlite3*> a_db)
    : m_db {a_db} {
    const std::scoped_lock lock {m_mutex};
    executeSql(m_db, "BEGIN TRANSACTION;");
  }

  // if we define a default constructor, we should define all of the default operations
  Transaction(const Transaction&) = delete;
  Transaction(Transaction&&) = delete;
  Transaction& operator=(const Transaction&) = delete;
  Transaction& operator=(Transaction&&) = delete;

  ~Transaction() {
    if (!m_didCommit) {
      try {
        executeSql(m_db, "ROLLBACK;");
      } catch (const DatabaseException& e) {
        // msvc declares std::exception::what() without noexcept
#if defined(_MSC_VER)
#  pragma warning(suppress : 26447) // a noexcept function calls a function that may throw
#endif // _MSC_VER
        LOG.error<"Error rollbacking transaction: {}">(e.what());
      } catch (...) { // can't throw in a destructor
        LOG.error<"Unknown Error rollbacking transaction.">();
      }
    }
  }

  void commit() {
    executeSql(m_db, "END TRANSACTION;");
    m_didCommit = true;
  }
}; // class Transaction

Database::Database()
  : Database(IN_MEMORY) {}

Database::Database(std::string_view name)
  : m_pImpl {std::make_unique<Implementation>(name)} {}

Database::~Database() {
  // a database can't be closed while it has prepared statements
  m_pImpl->m_inserts.reset();

  if (SQLITE_OK != sqlite3_close(m_pImpl->m_database)) {
    LOG.error<"Unknown error when closing the database. Fetching the error message...">();
    try {
      const auto pErrMsg = sqlite3_errmsg(m_pImpl->m_database);
      LOG.error<"Can't close the database file '{}: {}">(m_pImpl->m_dbName, pErrMsg);
    } catch (...) { // can't throw in a destructor
      LOG.error<"Couldn't fetch the error message.">();
    }
  }
}

/**
 * @throws DatabaseException if an error occurs during the insert
 */
void Database::save(const CashGame& game) {
  saveGame(*m_pImpl->m_inserts, game);
}

/**
 * @throws DatabaseException if an error occurs during the insert
 */
void Database::save(const Tournament& game) {
  saveGame(*m_pImpl->m_inserts, game);
}

void Database::save(std::span<const Player* const> players) {
  if (players.empty()) {
    return;
  }

  auto& insert = m_pImpl->m_inserts->player;
  std::ranges::for_each(players, [&insert](const auto* pPlayer) {
    insert.text(":playerName", pPlayer->getName())
        .text(":siteName", pPlayer->getSiteName())
        .boolean(":isHero", pPlayer->isHero())
        .text(":comments", pPlayer->getComments())
        .execute();
  });
}

/**
 * @throws DatabaseException if an error occurs during the insert
 */
static void saveSite(PreparedStatement& insert, const Site& s) {
  LOG.debug<"saving the Site with name={}">(s.getName());
  insert.text(":siteName", s.getName()).execute();
}

/**
 * Saves the given games one after the other: a SQLite connection executes one query at a time, so
 * saving them from several threads only made the threads wait, 38% slower on a whole history.
 * An error on one game is logged, and doesn't prevent saving the other ones.
 */
template <typename T>
static void saveGames(std::span<const T* const> games, Database& self) {
  std::ranges::for_each(games, [&self](const auto* pGame) {
    try {
      self.save(*pGame);
    } catch (const std::exception& e) {
      LOG.error<"Couldn't save the game with id='{}': {}">(pGame->getId(), e.what());
    } catch (...) {
      LOG.error<"Couldn't save the game with id='{}': unknown error">(pGame->getId());
    }
  });
}

/**
 * @throws DatabaseException if an error occurs during the insert
 */
void Database::save(const Site& site) {
  const std::scoped_lock lock {m_pImpl->m_saveMutex};
  Transaction transaction {m_pImpl->m_database};
  saveSite(m_pImpl->m_inserts->site, site);
  save(site.viewPlayers());
  const auto cashGames = site.viewCashGames();
  const auto tournaments = site.viewTournaments();
  saveGames(std::span {cashGames}, *this);
  saveGames(std::span {tournaments}, *this);
  transaction.commit();
}

Seat Database::getTableMaxSeat(std::string_view site, std::string_view table) const {
  static_assert(ps::contains(phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME, ':'),
                "ill-formed SQL template");
  PreparedStatement p {m_pImpl->m_database, phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME};
  p.text(":siteName", site).text(":tableName", table);

  if (QueryResult::NO_MORE_ROWS == p.nextRow()) {
    return Seat::seatUnknown;
  }

  validation::require(1 == p.getColumnCount(),
                      "bad number of columns in Database::getTableMaxSeat()");
  return tableSeat::fromInt(p.getColumnAsInt(0));
}

static std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS>
readTableStatisticsQuery(PreparedStatement& p) {
  static_assert(ps::contains(phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME, ':'),
                "ill-formed SQL template");
  std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS> playerStats {};

  if (QueryResult::NO_MORE_ROWS == p.nextRow()) {
    return playerStats;
  }

  do {
    validation::require(18 == p.getColumnCount(),
                        "bad number of columns in readTablePlayersStatistics()");
    const auto playerName = p.getColumnAsString(0);
    const auto siteName = p.getColumnAsString(1);
    const auto isHero = p.getColumnAsBool(2);
    const auto playerSeat = tableSeat::fromInt(p.getColumnAsInt(3));
    // const auto comment = gsl::at(columns, 4);
    const auto vpip = p.getColumnAsDouble(5);
    const auto pfr = p.getColumnAsDouble(6);
    const auto nbHands = p.getColumnAsInt(7);
    if (playerSeat != Seat::seatUnknown) {
      playerStats.at(tableSeat::toArrayIndex(playerSeat)) = std::make_unique<PlayerStatistics>(
          PlayerStatistics::Params {.playerName = playerName,
                                    .siteName = siteName,
                                    .isHero = isHero,
                                    .nbHands = nbHands,
                                    .vpip = vpip,
                                    .pfr = pfr,
                                    .limp = p.getColumnAsDouble(8),
                                    .nbFacedRaise = p.getColumnAsInt(9),
                                    .foldToRaise = p.getColumnAsDouble(10),
                                    .callRaise = p.getColumnAsDouble(11),
                                    .threeBet = p.getColumnAsDouble(12),
                                    .squeeze = p.getColumnAsDouble(13),
                                    .nbFacedThreeBet = p.getColumnAsInt(14),
                                    .foldToThreeBet = p.getColumnAsDouble(15),
                                    .callThreeBet = p.getColumnAsDouble(16),
                                    .fourBet = p.getColumnAsDouble(17)});
    } else {
      LOG.warn<"Invalid seat '{}' for player {}">(static_cast<int>(playerSeat), playerName);
    }
  } while (QueryResult::ONE_ROW_OR_MORE == p.nextRow());

  return playerStats;
}

std::unique_ptr<PlayerStatistics>
Database::readPlayerStatistics(std::string_view site, std::string_view playerName) const {
  static_assert(ps::contains(phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME, ':'),
                "ill-formed SQL template");
  PreparedStatement p {m_pImpl->m_database, phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME};
  p.text(":siteName", site).text(":playerName", playerName);

  if (QueryResult::NO_MORE_ROWS == p.nextRow()) {
    return nullptr;
  }

  validation::require(4 == p.getColumnCount(),
                      "bad number of columns in Database::readPlayerStatistics()");
  const auto isHero = 0 != p.getColumnAsInt(0);
  const auto vpip = p.getColumnAsDouble(1);
  const auto pfr = p.getColumnAsDouble(2);
  const auto nbHands = p.getColumnAsInt(3);
  return std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = playerName,
                                                                      .siteName = site,
                                                                      .isHero = isHero,
                                                                      .nbHands = nbHands,
                                                                      .vpip = vpip,
                                                                      .pfr = pfr});
}

TableStatistics Database::readTableStatistics(std::string_view site, std::string_view table) const {
  PreparedStatement p {m_pImpl->m_database, phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME};
  p.text(":siteName", site).text(":tableName", table);
  return TableStatistics {site, table, getTableMaxSeat(site, table), readTableStatisticsQuery(p)};
}

std::string Database::getDbName() const noexcept {
  return m_pImpl->m_dbName;
}

bool Database::isInMemory() const noexcept {
  return IN_MEMORY == getDbName();
}
