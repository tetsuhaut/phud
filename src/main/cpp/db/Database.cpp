#include "constants/TableConstants.hpp"
#include "db/Database.hpp" // DatabaseException, std::string, std::vector, std::ostream (specialized by fmt to use operator<< with Database), std::span
#include "db/SqlSelector.hpp"
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
#include "threads/ThreadPool.hpp" // ThreadPool, Future
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

    if (IN_MEMORY != name) {
      // the history is saved file by file: with the default settings, each commit waits for the
      // disk, 23 s for a whole history instead of 0.3 s. WAL stays consistent after a crash, and
      // only loses the last commits after a power failure, which a reimport recovers.
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
   * An insertion prepared once, then executed for each row. Building then parsing a SQL text for
   * each row took most of the import time. The values are bound, so they need no escaping: a
   * player name can contain a quote.
   * The values are bound in the order of the columns of the query, then execute() inserts the row:
   *   insert.text(handId).integer(index).execute();
   * The texts are not copied: they must stay valid until execute() returns.
   */
  class [[nodiscard]] PreparedInsert final {
  private:
    gsl::not_null<sqlite3*> m_db;
    std::string_view m_sql;
    // prepared at the first insertion: an existing database file can have no schema yet
    mutable sqlite3_stmt* m_pStatement {nullptr};
    mutable int m_nextPosition {1};

    void require(int returnCode, int expected, std::string_view what) const {
      if (expected != returnCode) {
        throw DatabaseException(fmt::format("Can't {} the query\n{}\nerror message:\n{}\n", what,
                                            m_sql, sqlite3_errmsg(m_db)));
      }
    }

    /**
     * @returns the statement, prepared if needed, and the position of the value to bind next.
     * @throws DatabaseException if the statement can't be prepared
     */
    [[nodiscard]] std::pair<sqlite3_stmt*, int> nextBinding() const {
      if (nullptr == m_pStatement) {
        require(sqlite3_prepare_v3(m_db, m_sql.data(), static_cast<int>(m_sql.size()),
                                   SQLITE_PREPARE_PERSISTENT, &m_pStatement, nullptr),
                SQLITE_OK, "prepare");
      }

      return {m_pStatement, m_nextPosition++};
    }

  public:
    PreparedInsert(gsl::not_null<sqlite3*> db, std::string_view sql)
      : m_db {db},
        m_sql {sql} {}

    PreparedInsert(const PreparedInsert&) = delete;
    PreparedInsert(PreparedInsert&&) = delete;
    PreparedInsert& operator=(const PreparedInsert&) = delete;
    PreparedInsert& operator=(PreparedInsert&&) = delete;

    // must be destroyed before the database is closed
    ~PreparedInsert() { sqlite3_finalize(m_pStatement); }

    const PreparedInsert& text(std::string_view value) const {
      const auto [pStatement, position] = nextBinding();
      require(sqlite3_bind_text(pStatement, position, value.data(),
                                static_cast<int>(value.size()), SQLITE_STATIC),
              SQLITE_OK, "bind a value of");
      return *this;
    }

    const PreparedInsert& integer(long long value) const {
      const auto [pStatement, position] = nextBinding();
      require(sqlite3_bind_int64(pStatement, position, value), SQLITE_OK, "bind a value of");
      return *this;
    }

    const PreparedInsert& real(double value) const {
      const auto [pStatement, position] = nextBinding();
      require(sqlite3_bind_double(pStatement, position, value), SQLITE_OK, "bind a value of");
      return *this;
    }

    // stored as 1 or 0
    const PreparedInsert& boolean(bool value) const { return integer(value ? 1 : 0); }

    /**
     * Inserts the row with the bound values, then gets ready for the next row.
     * @throws DatabaseException if not every value is bound, or if an error occurs
     */
    void execute() const {
      const auto nbBoundValues = m_nextPosition - 1;
      m_nextPosition = 1;
      require(nbBoundValues, sqlite3_bind_parameter_count(m_pStatement), "bind every value of");
      const auto returnCode = sqlite3_step(m_pStatement);
      // reset even after an error, so that the statement can be used again
      sqlite3_reset(m_pStatement);
      require(returnCode, SQLITE_DONE, "execute");
    }
  }; // class PreparedInsert

  /**
   * Every insertion of the database, prepared once for the database connection.
   */
  struct [[nodiscard]] PreparedInserts final {
    PreparedInsert site;
    PreparedInsert player;
    PreparedInsert game;
    PreparedInsert tournament;
    PreparedInsert cashGame;
    PreparedInsert hand;
    PreparedInsert handPlayer;
    PreparedInsert handPlayerStats;
    PreparedInsert cashGameHand;
    PreparedInsert tournamentHand;
    PreparedInsert action;

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
    [[nodiscard]] const PreparedInsert& gameHand(GameType gameType) const noexcept {
      return (GameType::tournament == gameType) ? tournamentHand : cashGameHand;
    }
  }; // struct PreparedInserts

  void insertGame(const PreparedInsert& insert, const auto& game) {
    const auto startDate = game.getStartDate().toSqliteDate(); // must live until execute()
    insert.text(game.getId()) // use the tournament ID for the Game
        .text(game.getSiteName())
        .text(game.getName())
        .text(toString(game.getVariant()))
        .text(toString(game.getLimitType()))
        .boolean(game.isRealMoney())
        .text(tableSeat::toString(game.getMaxNbSeats()))
        .text(startDate)
        .execute();
  }

  void insertSpecificGame(const PreparedInserts& inserts, const Tournament& tournament) {
    LOG.debug<"saving the tournament with id={}">(tournament.getId());
    inserts.tournament.text(tournament.getId()).real(tournament.getBuyIn()).execute();
  }

  void insertSpecificGame(const PreparedInserts& inserts, const CashGame& cashGame) {
    LOG.debug<"saving the cash game with id={}">(cashGame.getId());
    inserts.cashGame.text(cashGame.getId())
        .real(cashGame.getSmallBlind())
        .real(cashGame.getBigBlind())
        .execute();
  }

  void insertHand(const PreparedInsert& insert, const Hand& hand) {
    const auto startDate = hand.getStartDate().toSqliteDate(); // must live until execute()
    insert.text(hand.getId())
        .text(hand.getSiteName())
        .text(hand.getTableName())
        .text(tableSeat::toString(hand.getButtonSeat()))
        .text(tableSeat::toString(hand.getMaxSeats()))
        .integer(hand.getAnte())
        .integer(hand.getLevel())
        .text(startDate)
        .text(toString(hand.getHeroCard1()))
        .text(toString(hand.getHeroCard2()))
        .text(toString(hand.getHeroCard3()))
        .text(toString(hand.getHeroCard4()))
        .text(toString(hand.getHeroCard5()))
        .text(toString(hand.getBoardCard1()))
        .text(toString(hand.getBoardCard2()))
        .text(toString(hand.getBoardCard3()))
        .text(toString(hand.getBoardCard4()))
        .text(toString(hand.getBoardCard5()))
        .execute();
  }

  void insertHandPlayers(const PreparedInsert& insert, const Hand& hand) {
    const auto seats = hand.getSeats();
    validation::require(!std::ranges::all_of(seats, [](const auto& p) { return p.empty(); }),
                        "trying to save a hand with no players");
    const auto isSeated = [](const auto& indexedPlayer) {
      return !std::get<1>(indexedPlayer).empty();
    };

    // we suppose the Player has been saved before this call
    for (const auto& [index, playerName] : seats | std::views::enumerate |
                                               std::views::filter(isSeated)) {
      insert.text(hand.getId())
          .text(playerName)
          .text(tableSeat::toString(tableSeat::fromArrayIndex(index)))
          .boolean(hand.isWinner(playerName))
          .execute();
    }
  }

  void insertHandPlayerStats(const PreparedInsert& insert, const Hand& hand,
                             const HandPlayerStats& stats) {
    insert.text(hand.getId())
        .text(stats.playerName)
        .text(hand.getSiteName())
        .text(toString(hand.getGameType()))
        .boolean(stats.vpip)
        .boolean(stats.pfr)
        .boolean(stats.limp)
        .boolean(stats.facedRaise)
        .boolean(stats.foldToRaise)
        .boolean(stats.callRaise)
        .boolean(stats.threeBet)
        .boolean(stats.couldSqueeze)
        .boolean(stats.squeeze)
        .boolean(stats.facedThreeBet)
        .boolean(stats.foldToThreeBet)
        .boolean(stats.callThreeBet)
        .boolean(stats.fourBet)
        .execute();
  }

  void insertAction(const PreparedInsert& insert, const Hand& hand, const Action& action) {
    insert.text(toString(action.getStreet()))
        .text(hand.getId())
        .text(action.getPlayerName())
        .text(toString(action.getType()))
        .integer(static_cast<long long>(action.getIndex()))
        .real(action.getAmount())
        .execute();
  }

  /**
   * Saves a hand, its players, their statistics and actions, and links it to its game.
   * @throws DatabaseException if an error occurs during the insert
   */
  void saveHand(const PreparedInserts& inserts, std::string_view gameId, const Hand& hand) {
    insertHand(inserts.hand, hand);
    insertHandPlayers(inserts.handPlayer, hand);
    std::ranges::for_each(computeHandPlayerStats(hand), [&inserts, &hand](const auto& stats) {
      insertHandPlayerStats(inserts.handPlayerStats, hand, stats);
    });
    inserts.gameHand(hand.getGameType()).text(gameId).text(hand.getId()).execute();
    LOG.trace<"saving {} actions from hand with id={}">(hand.viewActions().size(), hand.getId());
    std::ranges::for_each(hand.viewActions(), [&inserts, &hand](const auto* pAction) {
      insertAction(inserts.action, hand, *pAction);
    });
  }

  /**
   * @throws DatabaseException if an error occurs during the insert
   */
  void saveGame(const PreparedInserts& inserts, const auto& game) {
    const auto hands = game.viewHands();
    const auto gameId = game.getId();
    insertGame(inserts.game, game);
    insertSpecificGame(inserts, game);
    LOG.debug<"saving {} hands from the game with id={}">(hands.size(), gameId);
    std::ranges::for_each(hands, [&inserts, &gameId](const auto* pHand) {
      saveHand(inserts, gameId, *pHand);
    });
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
void Database::save(const CashGame& game) const {
  saveGame(*m_pImpl->m_inserts, game);
}

/**
 * @throws DatabaseException if an error occurs during the insert
 */
void Database::save(const Tournament& game) const {
  saveGame(*m_pImpl->m_inserts, game);
}

void Database::save(std::span<const Player* const> players) const {
  if (players.empty()) {
    return;
  }

  const auto& insert = m_pImpl->m_inserts->player;
  std::ranges::for_each(players, [&insert](const auto* pPlayer) {
    insert.text(pPlayer->getName())
        .text(pPlayer->getSiteName())
        .boolean(pPlayer->isHero())
        .text(pPlayer->getComments())
        .execute();
  });
}

/**
 * @throws DatabaseException if an error occurs during the insert
 */
static void saveSite(const PreparedInsert& insert, const Site& s) {
  LOG.debug<"saving the Site with name={}">(s.getName());
  insert.text(s.getName()).execute();
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

enum class /*[[nodiscard]]*/ QueryResult : std::uint8_t { NO_MORE_ROWS, ONE_ROW_OR_MORE };

class [[nodiscard]] PreparedStatement final {
private:
  sqlite3_stmt* m_pStatement = nullptr;
  gsl::not_null<sqlite3*> m_pDatabase;
  std::string m_sql;

public:
  PreparedStatement(const gsl::not_null<sqlite3*> pDatabase, std::string_view sql)
    : m_pDatabase {pDatabase},
      m_sql {sql} {
    if (SQLITE_OK != sqlite3_prepare_v2(m_pDatabase, sql.data(), -1, &m_pStatement, nullptr)) {
      throw DatabaseException(
          fmt::format("Can't prepare the statement for query:\n{}\nDatabase error is:\n{}", m_sql,
                      sqlite3_errmsg(m_pDatabase)));
    }
  }

  // if we define a default constructor, we should define all of the default operations
  PreparedStatement(const PreparedStatement&) = delete;
  PreparedStatement(PreparedStatement&&) = delete;
  PreparedStatement& operator=(const PreparedStatement&) = delete;
  PreparedStatement& operator=(PreparedStatement&&) = delete;

  ~PreparedStatement() {
    if (SQLITE_OK != sqlite3_finalize(m_pStatement)) {
      try {
        LOG.error<"Can't close a prepared statement, database error is:\n{}">(
            sqlite3_errmsg(m_pDatabase));
      } catch (...) { // can't throw in a destructor
        LOG.error<"Can't close a prepared statement, database error is unknown.">();
      }
    }
  }

  /**
   * @returns QueryResult::ONE_ROW_OR_MORE when the executed SQL prepared statement returned one or
   * more rows. else returns QueryResult::NO_MORE_ROWS.
   * @throws DatabaseException if an error occurs
   */
  [[nodiscard]] QueryResult execute() {
    const auto ret = sqlite3_step(m_pStatement);

    if (SQLITE_DONE == ret) {
      return QueryResult::NO_MORE_ROWS;
    }

    if (SQLITE_ROW == ret) {
      return QueryResult::ONE_ROW_OR_MORE;
    }

    throw DatabaseException(
        fmt::format("Can't execute the prepared statement\n{}\nDatabase error is:\n{}", m_sql,
                    sqlite3_errmsg(m_pDatabase)));
  }

  [[nodiscard]] int getColumnCount() const noexcept { return sqlite3_column_count(m_pStatement); }
  [[nodiscard]] int getColumnAsInt(int column) const noexcept {
    return sqlite3_column_int(m_pStatement, column);
  }

  [[nodiscard]] double getColumnAsDouble(int column) const noexcept {
    return sqlite3_column_double(m_pStatement, column);
  }

  [[nodiscard]] std::string getColumnAsString(int column) {
    if (const auto* text {sqlite3_column_text(m_pStatement, column)}; nullptr != text) {
      return reinterpret_cast<const char*>(text);
    }
    throw DatabaseException(
        fmt::format("Got a null column name for column {} preparing statement {}", column, m_sql));
  }

  [[nodiscard]] bool getColumnAsBool(int column) const noexcept {
    return 0 != getColumnAsInt(column);
  }
}; // class PreparedStatement

Seat Database::getTableMaxSeat(std::string_view site, std::string_view table) const {
  static_assert(ps::contains(phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME, '?'),
                "ill-formed SQL template");
  const auto& sql {SqlSelector(phud::sql::GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME)
                       .site(site)
                       .table(table)
                       .toString()};
  PreparedStatement p {m_pImpl->m_database, sql};

  if (QueryResult::NO_MORE_ROWS == p.execute()) {
    return Seat::seatUnknown;
  }

  validation::require(1 == p.getColumnCount(),
                      "bad number of columns in Database::getTableMaxSeat()");
  return tableSeat::fromInt(p.getColumnAsInt(0));
}

static std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS>
readTableStatisticsQuery(PreparedStatement& p) {
  static_assert(ps::contains(phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME, '?'),
                "ill-formed SQL template");
  std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS> playerStats {};

  if (QueryResult::NO_MORE_ROWS == p.execute()) {
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
      playerStats.at(tableSeat::toArrayIndex(playerSeat)) =
          std::make_unique<PlayerStatistics>(PlayerStatistics::Params {
              .playerName = playerName,
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
  } while (QueryResult::ONE_ROW_OR_MORE == p.execute());

  return playerStats;
}

std::unique_ptr<PlayerStatistics>
Database::readPlayerStatistics(std::string_view site, std::string_view playerName) const {
  static_assert(ps::contains(phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME, '?'),
                "ill-formed SQL template");
  const auto& sql {SqlSelector(phud::sql::GET_STATS_BY_SITE_AND_PLAYER_NAME)
                       .site(site)
                       .player(playerName)
                       .toString()};
  PreparedStatement p {m_pImpl->m_database, sql};

  if (QueryResult::NO_MORE_ROWS == p.execute()) {
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
  const auto& sql {SqlSelector(phud::sql::GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME)
                       .site(site)
                       .table(table)
                       .toString()};
  PreparedStatement p {m_pImpl->m_database, sql};
  return TableStatistics {site, table, getTableMaxSeat(site, table), readTableStatisticsQuery(p)};
}

std::string Database::getDbName() const noexcept {
  return m_pImpl->m_dbName;
}

bool Database::isInMemory() const noexcept {
  return IN_MEMORY == getDbName();
}
