#include "db/Database.hpp"  // DatabaseException, std::string, std::vector, std::ostream (specialized by fmt to use operator<< with Database)
#include "db/SqlInsertor.hpp"  // Game
#include "db/SqlSelector.hpp" // phudAssert
#include "db/sqliteQueries.hpp" // all the SQL queries
#include "entities/Action.hpp"
#include "entities/Game.hpp" // Cashgame, Limit, Time, Tournament, Variant
#include "entities/GameType.hpp"
#include "entities/Hand.hpp"  // std::array
#include "entities/Player.hpp"  // Player
#include "entities/Site.hpp"  // Site
#include "filesystem/Filesystem.hpp" // phud::filesystem
#include "log/Logger.hpp" // fmt::format(), CURRENT_FILE_NAME
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadPool.hpp"  // ThreadPool
#include "entities/Card.hpp" // Card, String
#include <frozen/unordered_map.h>
#include <gsl/gsl> // gsl::not_null
#include <sqlite3.h>  // sqlite3*
#include <stlab/concurrency/utility.hpp> // stlab::await, Future

#include <mutex>
#include <ranges>
#include <span>

// from sqlite3.h: 'The application does not need to worry about freeing the result.' So no need to
// free the char* returned by sqlite3_errmsg().

// FUTURE: the read query name, content, entity type, and callback should be linked.
// FUTURE: use a cache

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;
namespace pf = phud::filesystem;
namespace ps = phud::strings;

static constexpr std::string_view IN_MEMORY { ":memory:" };

/**
 * Opens the database, this will create the database file if needed.
 * @throws DatabaseException in case of error
 */
static inline gsl::not_null<sqlite3*> openDatabase(std::string_view dbName) {
  sqlite3* pDb { nullptr };

  if (SQLITE_OK != sqlite3_open(dbName.data(), &pDb)) {
    throw DatabaseException(fmt::format("Can't open database file '{}': {}",
                                        dbName, sqlite3_errmsg(pDb)));
  }

  return pDb;
}

/**
 * Runs a query against the database.
 * @throws DatabaseException if an error occurs
 */
static inline void executeSql(const gsl::not_null<sqlite3*> pDb, std::string_view sql) {
  LOG.trace(sql);

  if (SQLITE_OK != sqlite3_exec(pDb,
                                sql.data(), /*callback*/nullptr, /*callbackParam*/nullptr, /*errorMsg*/nullptr)) {
    throw DatabaseException(fmt::format(
                              "Can't execute the query\n{}\nerror message:\n{}\n",
                              sql, sqlite3_errmsg(pDb)));
  }
}

/**
* @throws DatabaseException in case of problem getting the query SQL code or opening the database
* file
*/
[[nodiscard]] static inline gsl::not_null<sqlite3*> createDatabase(std::string_view name) {
  phudAssert(!name.empty(), "db name is empty !!!");
  fs::path dbFile { name };
  const auto isDbCreation { !pf::isFile(dbFile) };

  if (isDbCreation) {
    LOG.info<"Creating the database {}">(IN_MEMORY == name ? "in memory" : pf::absolute(
                                           dbFile).string());
  } else { LOG.info<"Opening the existing database file {}, no schema created.">(pf::absolute(dbFile).string()); }

  const auto& pDb { openDatabase(name) }; // will create the database file if needed

  if (isDbCreation) {
    LOG.info<"creating the database schema {}">(name);
    std::ranges::for_each(phud::sql::CREATE_QUERIES, [pDb](const auto & query) { executeSql(pDb, query); });
    LOG.info<"database created">();
  }

  return pDb;
}

struct [[nodiscard]] Database::Implementation final {
  std::string m_dbName;
  gsl::not_null<sqlite3*> m_database;
  
  explicit Implementation(std::string_view dbName)
    : m_dbName { dbName },
      m_database { createDatabase(dbName) }
  {}
}; // struct  Database::Implementation

class [[nodiscard]] Transaction final {
private:
  std::mutex m_mutex {};
  gsl::not_null<sqlite3*> m_db;
  bool m_didCommit { false };

public:
  explicit Transaction(gsl::not_null<sqlite3*> a_db)
    : m_db { a_db } {
    const std::lock_guard<std::mutex> lock { m_mutex };
    executeSql(m_db, "BEGIN TRANSACTION;");
  }

  // if we define a default constructor, we should define all of the default operations
  Transaction(const Transaction&) = delete;
  Transaction(Transaction&&) = delete;
  Transaction& operator=(const Transaction&) = delete;
  Transaction& operator=(Transaction&&) = delete;

  ~Transaction() {
    if (!m_didCommit) {
      // can't throw in a destructor
      try {
        executeSql(m_db, "ROLLBACK;");
      } catch (...) {
        std::exit(1);
      }
    }
  }

  void commit() {
    executeSql(m_db, "END TRANSACTION;");
    m_didCommit = true;
  }
}; // class Transaction

static_assert(ps::contains(phud::sql::INSERT_CASHGAME_HAND, '?'), "ill-formed SQL template");
static_assert(ps::contains(phud::sql::INSERT_TOURNAMENT_HAND, '?'), "ill-formed SQL template");
static constexpr auto GAME_TYPE_TO_ID_AND_COLUMN_NAME {
  frozen::make_unordered_map<GameType, std::string_view>({
    {GameType::cashGame, phud::sql::INSERT_CASHGAME_HAND},
    {GameType::tournament, phud::sql::INSERT_TOURNAMENT_HAND }
  })
};

static inline void insertGame(const gsl::not_null<sqlite3*> db, const auto& g) {
  static_assert(ps::contains(phud::sql::INSERT_GAME, '?'), "ill-formed SQL template");
  executeSql(db, SqlInsertor(phud::sql::INSERT_GAME)
             .gameId(g.getId()) // use the tournament ID for the Game
             .siteName(g.getSiteName())
             .gameName(g.getName())
             .variant(g.getVariant())
             .limitType(g.getLimitType())
             .isRealMoney(g.isRealMoney())
             .nbMaxSeats(g.getMaxNbSeats())
             .startDate(g.getStartDate().toSqliteDate())
             .build());
}

static inline void insertSpecificGame(const gsl::not_null<sqlite3*> db, const Tournament& t) {
  LOG.info<"saving the tournament with id={}">(t.getId());
  static_assert(ps::contains(phud::sql::INSERT_TOURNAMENT, '?'), "ill-formed SQL template");
  executeSql(db, SqlInsertor(phud::sql::INSERT_TOURNAMENT)
             .tournamentId(t.getId())
             .buyIn(t.getBuyIn())
             .build());
}

/**
* @throws DatabaseException
*/
static inline void insertSpecificGame(const gsl::not_null<sqlite3*> db, const CashGame& cg) {
  LOG.info<"saving the cash game with id={}">(cg.getId());
  static_assert(ps::contains(phud::sql::INSERT_CASHGAME, '?'), "ill-formed SQL template");
  executeSql(db, SqlInsertor(phud::sql::INSERT_CASHGAME)
             .cashGameId(cg.getId())
             .smallBlind(cg.getSmallBlind())
             .bigBlind(cg.getBigBlind())
             .build());
}

/**
 * @throws DatabaseException
 */
static inline void saveActions(const gsl::not_null<sqlite3*> db,
                               std::span<const Action* const> actions) {
  if (actions.empty()) { return; }

  static_assert(ps::contains(phud::sql::INSERT_ACTION, '?'), "ill-formed SQL template");
  SqlInsertor query { phud::sql::INSERT_ACTION };
  std::ranges::for_each(actions, [&query](const auto & pAction) {
    query
    .street(pAction->getStreet())
    .handId(pAction->getHandId())
    .playerName(pAction->getPlayerName())
    .actionType(pAction->getType())
    .actionIndex(pAction->getIndex())
    .betAmount(pAction->getBetAmount())
    .newInsert();
  });
  executeSql(db, query.build());
}

/**
 * @throws DatabaseException
 */
static inline void saveGame(const gsl::not_null<sqlite3*> db, const auto& game) {
  const auto& hands { game.viewHands() };
  const auto& gameId { game.getId() };
  insertGame(db, game);
  insertSpecificGame(db, game);
  LOG.info<"saving {} hands from the game with id={}">(hands.size(), gameId);
  saveHands(db, gameId, hands);
  std::ranges::for_each(hands, [db](const auto & h) {
    LOG.trace<"saving {} actions from hand with id={}">(h->viewActions().size(), h->getId());
    auto av { h->viewActions() };
    saveActions(db, av);
  });
}

Database::Database() : Database(IN_MEMORY) {}

Database::Database(std::string_view dbName)
  : m_pImpl { std::make_unique<Implementation>(dbName) } {
}

Database::~Database() {
  if (SQLITE_OK != sqlite3_close(m_pImpl->m_database)) {
    // can't throw in a destructor
    try {
      LOG.error<"Can't close the database file '{}: {}">(m_pImpl->m_dbName,
          sqlite3_errmsg(m_pImpl->m_database));
    } catch (...) {
      std::exit(2);
    }
  }
}

/**
 * @throws DatabaseException
 */
void Database::save(const CashGame& game) { saveGame(m_pImpl->m_database, game); }

/**
 * @throws DatabaseException
 */
void Database::save(const Tournament& game) { saveGame(m_pImpl->m_database, game); }

void Database::save(std::span<const Player* const> players) {
  if (players.empty()) { return; }

  static_assert(ps::contains(phud::sql::INSERT_PLAYER, '?'), "ill-formed SQL template");
  SqlInsertor query { phud::sql::INSERT_PLAYER };
  std::ranges::for_each(players, [&query](const auto & p) {
    query
    .playerName(p->getName())
    .siteName(p->getSiteName())
    .isHero(p->isHero())
    .comments(p->getComments())
    .newInsert();
  });
  executeSql(m_pImpl->m_database, query.build());
}

/**
* @throws DatabaseException
*/
static inline void save(const gsl::not_null<sqlite3*> pDb, const Site& s) {
  LOG.info<"saving the Site with name={}">(s.getName());
  static_assert(ps::contains(phud::sql::INSERT_SITE, '?'), "ill-formed SQL template");
  executeSql(pDb, SqlInsertor(phud::sql::INSERT_SITE).siteName(s.getName()).build());
}

template<typename T>
static inline std::vector<Future<void>> saveGamesAsync(const std::vector<const T*>& games, Database& self) {
  std::vector<Future<void>> ret;
  ret.reserve(games.size());
  std::transform(games.cbegin(), games.cend(), std::back_inserter(ret), [&](const auto& pGame) {
    return ThreadPool::submit([&]() {
      const auto& gameId { pGame->getId() };

      try {
        self.save(*pGame);
      } catch (const std::exception& e) {
        LOG.error<"Couldn't save the game with id='{}': {}">(gameId, e.what());
      } catch (...) {
        LOG.error<"Couldn't save the game with id='{}': unknown error">(gameId);
      }
    });
  });
  return ret;
}

/**
 * @throws DatabaseException
 */
void Database::save(const Site& site) {
  Transaction transaction { m_pImpl->m_database };
  ::save(m_pImpl->m_database, site);
  save(site.viewPlayers());
  const auto& cashGames { site.viewCashGames() };
  const auto& tournaments { site.viewTournaments() };
  auto tasks1 { saveGamesAsync(cashGames, *this) };
  auto tasks2 = saveGamesAsync(tournaments, *this);
  std::ranges::for_each(tasks1, [](auto & task) { stlab::await(std::move(task)); });
  std::ranges::for_each(tasks2, [](auto & task) { stlab::await(std::move(task)); });
  transaction.commit();
}

/*static inline*/ void insertHand(SqlInsertor& handInsert, const Hand& hand) {
  handInsert
  .handId(hand.getId())
  .siteName(hand.getSiteName())
  .tableName(hand.getTableName())
  .buttonSeat(hand.getButtonSeat())
  .maxSeats(hand.getMaxSeats())
  .level(hand.getLevel())
  .ante(hand.getAnte())
  .startDate(hand.getStartDate().toSqliteDate())
  .heroCard1(hand.getHeroCard1())
  .heroCard2(hand.getHeroCard2())
  .heroCard3(hand.getHeroCard3())
  .heroCard4(hand.getHeroCard4())
  .heroCard5(hand.getHeroCard5())
  .boardCard1(hand.getBoardCard1())
  .boardCard2(hand.getBoardCard2())
  .boardCard3(hand.getBoardCard3())
  .boardCard4(hand.getBoardCard4())
  .boardCard5(hand.getBoardCard5())
  .newInsert();
}

/**
 * @throws DatabaseException
 */
static inline void saveHands(const gsl::not_null<sqlite3*> db, std::string_view gameId,
                             std::span<const Hand* const> hands) {
  if (hands.empty()) { return; }

  LOG.trace<"create insert queries">();
  static_assert(ps::contains(phud::sql::INSERT_HAND, '?'), "ill-formed SQL template");
  SqlInsertor handInsert { phud::sql::INSERT_HAND };
  static_assert(ps::contains(phud::sql::INSERT_HAND_PLAYER, '?'), "ill-formed SQL template");
  SqlInsertor handPlayerInsert { phud::sql::INSERT_HAND_PLAYER };
  SqlInsertor gameHandInsert { GAME_TYPE_TO_ID_AND_COLUMN_NAME.find(gsl::at(hands, 0)->getGameType())->second };
  std::ranges::for_each(hands, [&](const auto & pHand) {
    insertHand(handInsert, *pHand);
    const auto& seats { pHand->getSeats() };
    phudAssert(!std::ranges::all_of(seats, [](const auto & p) { return p.empty(); }),
    "trying to save a hand with no players");
    int i { 1 };
    std::ranges::for_each(seats, [&](const auto & playerName) {
      // we suppose the Player has been saved before this call
      if (!playerName.empty()) {
        handPlayerInsert
        .handId(pHand->getId())
        .playerName(playerName)
        .playerSeat(tableSeat::fromInt(i))
        .isWinner(pHand->isWinner(playerName))
        .newInsert();
      }

      i++;
    });
    gameHandInsert
    .gameId(gameId)
    .handId(pHand->getId())
    .newInsert();
  });
  LOG.trace<"run insert queries">();
  LOG.info<"insert hands for table {}">(hands.front()->getTableName());
  executeSql(db, handInsert.build());
  executeSql(db, handPlayerInsert.build());
  executeSql(db, gameHandInsert.build());
  LOG.trace<"exit saveHands()">();
}

enum class /*[[nodiscard]]*/ QueryResult : short { NO_MORE_ROWS, ONE_ROW_OR_MORE };

class [[nodiscard]] PreparedStatement final {
private:
  sqlite3_stmt* m_pStatement { nullptr };
  gsl::not_null<sqlite3*> m_pDatabase;
  std::string m_sql;

public:
  PreparedStatement(const gsl::not_null<sqlite3*> pDatabase, std::string_view sql)
    : m_pDatabase { pDatabase }, m_sql { sql } {
    if (SQLITE_OK != sqlite3_prepare_v2(m_pDatabase, sql.data(), -1, &m_pStatement, nullptr)) {
      throw DatabaseException(fmt::format(
                                "Can't prepare the statement for query:\n{}\nDatabase error is:\n{}",
                                m_sql, sqlite3_errmsg(m_pDatabase)));
    }
  }

  // if we define a default constructor, we should define all of the default operations
  PreparedStatement(const PreparedStatement&) = delete;
  PreparedStatement(PreparedStatement&&) = delete;
  PreparedStatement& operator=(const PreparedStatement&) = delete;
  PreparedStatement& operator=(PreparedStatement&&) = delete;

  ~PreparedStatement() {
    if (SQLITE_OK != sqlite3_finalize(m_pStatement)) {
      // can't throw in a destructor
      try {
        LOG.error<"Can't close a prepared statement, database error is:\n{}">(sqlite3_errmsg(m_pDatabase));
      } catch (...) {
        std::exit(3);
      }
    }
  }

  /**
  * @returns QueryResult::ONE_ROW_OR_MORE when the executed SQL prepared statement returned one or more rows.
  * else returns QueryResult::NO_MORE_ROWS.
  * @throws DatabaseException if an error occurs
  */
  [[nodiscard]] QueryResult execute() {
    const auto ret { sqlite3_step(m_pStatement) };

    if (SQLITE_DONE == ret) { return QueryResult::NO_MORE_ROWS; }

    if (SQLITE_ROW == ret) { return QueryResult::ONE_ROW_OR_MORE; }

    throw DatabaseException(fmt::format(
                              "Can't execute the prepared statement\n{}\nDatabase error is:\n{}",
                              m_sql, sqlite3_errmsg(m_pDatabase)));
  }

  [[nodiscard]] int getColumnCount() noexcept { return sqlite3_column_count(m_pStatement); }
  [[nodiscard]] int getColumnAsInt(int column) noexcept { return sqlite3_column_int(m_pStatement, column); }
  [[nodiscard]] double getColumnAsDouble(int column) noexcept { return sqlite3_column_double(m_pStatement, column); }
  [[nodiscard]] std::string getColumnAsString(int column) {
    return reinterpret_cast<const char*>(sqlite3_column_text(m_pStatement, column));
  }
  [[nodiscard]] bool getColumnAsBool(int column) noexcept { return 0 != getColumnAsInt(column); }
}; // class PreparedStatement

Seat Database::getTableMaxSeat(std::string_view site, std::string_view table) const {
  static_assert(ps::contains(phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME, '?'),
                "ill-formed SQL template");
  const auto& sql { SqlSelector(phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME)
                    .site(site)
                    .table(table)
                    .toString() };
  PreparedStatement p { m_pImpl->m_database, sql };

  if (QueryResult::NO_MORE_ROWS == p.execute()) { return Seat::seatUnknown; }

  phudAssert(1 == p.getColumnCount(), "bad number of columns in Database::getTableMaxSeat()");
  return tableSeat::fromInt(p.getColumnAsInt(0));
}

static std::array<std::unique_ptr<PlayerStatistics>, 10> readTableStatisticsQuery(PreparedStatement& p) {
  static_assert(ps::contains(phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME, '?'),
                "ill-formed SQL template");
  std::array<std::unique_ptr<PlayerStatistics>, 10> playerStats {};

  if (QueryResult::NO_MORE_ROWS == p.execute()) { return playerStats; }

  do {
    phudAssert(8 == p.getColumnCount(), "bad number of columns in readTablePlayersStatistics()");
    const auto& playerName { p.getColumnAsString(0) };
    const auto& siteName { p.getColumnAsString(1) };
    const auto isHero { p.getColumnAsBool(2) };
    const auto playerSeat { tableSeat::fromInt(p.getColumnAsInt(3)) };
    // const auto& comment { gsl::at(columns, 4) };
    const auto vpip { p.getColumnAsDouble(5) };
    const auto pfr { p.getColumnAsDouble(6) };
    const auto nbHands { p.getColumnAsInt(7) };
    playerStats.at(tableSeat::toArrayIndex(playerSeat)) = std::make_unique<PlayerStatistics>
    (PlayerStatistics::Params {
      .playerName = playerName, .siteName = siteName, .isHero = isHero, .nbHands = nbHands, .vpip = vpip, .pfr = pfr });
  } while (QueryResult::ONE_ROW_OR_MORE == p.execute());

  return playerStats;
}

TableStatistics Database::readTableStatistics(const ReadTableStatisticsArgs& args) const {
  const auto& sql { SqlSelector(phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME)
                    .site(args.site)
                    .table(args.table)
                    .toString() };
  PreparedStatement p { m_pImpl->m_database, sql };
  return { .m_maxSeats = getTableMaxSeat(args.site, args.table), .m_tableStats = readTableStatisticsQuery(p) };
}

std::unique_ptr<PlayerStatistics> Database::readPlayerStatistics(std::string_view site, std::string_view player) const {
  static_assert(ps::contains(phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME, '?'),
                "ill-formed SQL template");
  const auto& sql { SqlSelector(phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME).site(site)
                    .player(player).toString() };
  PreparedStatement p { m_pImpl->m_database, sql };

  if (QueryResult::NO_MORE_ROWS == p.execute()) { return nullptr; }

  phudAssert(4 == p.getColumnCount(), "bad number of columns in Database::readPlayerStatistics()");
  const auto isHero { 0 != p.getColumnAsInt(0) };
  const auto vpip { p.getColumnAsDouble(1) };
  const auto pfr { p.getColumnAsDouble(2) };
  const auto nbHands { p.getColumnAsInt(3) };
  return std::make_unique<PlayerStatistics>(
           PlayerStatistics::Params {.playerName = player, .siteName = site, .isHero = isHero, .nbHands = nbHands, .vpip = vpip, .pfr = pfr});
}

bool Database::isInMemory() const noexcept { return IN_MEMORY == getDbName(); }

std::string Database::getDbName() const noexcept { return m_pImpl->m_dbName; }
