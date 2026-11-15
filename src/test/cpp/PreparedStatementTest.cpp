#include "TestInfrastructure.hpp"
#include "db/Database.hpp" // DatabaseException
#include "db/PreparedStatement.hpp"
#include <gsl/gsl> // gsl::finally
#include <sqlite3.h>
#include <string>

namespace {
  constexpr std::string_view INSERT_ROW {
      "INSERT INTO Row (t, i, r, b) VALUES (:text, :integer, :real, :boolean);"};
  constexpr std::string_view SELECT_ROWS {"SELECT t, i, r, b FROM Row WHERE t = :text ORDER BY i;"};

  /**
   * An in-memory database with a table Row having a column of each type, created by each test.
   */
  struct [[nodiscard]] InMemoryDatabase final {
    sqlite3* m_pDb {nullptr};

    InMemoryDatabase() {
      BOOST_REQUIRE(SQLITE_OK == sqlite3_open(":memory:", &m_pDb));
      execute("CREATE TABLE Row (t TEXT NOT NULL, i INTEGER UNIQUE, r REAL, b INTEGER);");
    }

    InMemoryDatabase(const InMemoryDatabase&) = delete;
    InMemoryDatabase(InMemoryDatabase&&) = delete;
    InMemoryDatabase& operator=(const InMemoryDatabase&) = delete;
    InMemoryDatabase& operator=(InMemoryDatabase&&) = delete;

    // declared before the PreparedStatement objects of a test, so destroyed after them: the
    // database must be closed last
    ~InMemoryDatabase() { sqlite3_close(m_pDb); }

    [[nodiscard]] gsl::not_null<sqlite3*> get() const { return m_pDb; }

    void execute(std::string_view sql) const {
      BOOST_REQUIRE(SQLITE_OK ==
                    sqlite3_exec(m_pDb, std::string(sql).c_str(), nullptr, nullptr, nullptr));
    }

    [[nodiscard]] int countRows() const {
      sqlite3_stmt* pStatement {nullptr};
      BOOST_REQUIRE(SQLITE_OK == sqlite3_prepare_v2(m_pDb, "SELECT COUNT(*) FROM Row;", -1,
                                                    &pStatement, nullptr));
      const auto _ {gsl::finally([pStatement] { sqlite3_finalize(pStatement); })};
      BOOST_REQUIRE(SQLITE_ROW == sqlite3_step(pStatement));
      return sqlite3_column_int(pStatement, 0);
    }

    struct [[nodiscard]] Row final {
      std::string text;
      long long integer;
      double real;
      long long boolean;
    };

    /**
     * @returns the row having the given value in the column i
     */
    [[nodiscard]] Row readRow(long long i) const {
      sqlite3_stmt* pStatement {nullptr};
      BOOST_REQUIRE(SQLITE_OK == sqlite3_prepare_v2(m_pDb,
                                                    "SELECT t, i, r, b FROM Row WHERE i = ?;", -1,
                                                    &pStatement, nullptr));
      const auto _ {gsl::finally([pStatement] { sqlite3_finalize(pStatement); })};
      BOOST_REQUIRE(SQLITE_OK == sqlite3_bind_int64(pStatement, 1, i));
      BOOST_REQUIRE(SQLITE_ROW == sqlite3_step(pStatement));
      return {reinterpret_cast<const char*>(sqlite3_column_text(pStatement, 0)),
              sqlite3_column_int64(pStatement, 1), sqlite3_column_double(pStatement, 2),
              sqlite3_column_int64(pStatement, 3)};
    }
  }; // struct InMemoryDatabase
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(PreparedStatementTest)

BOOST_AUTO_TEST_CASE(PreparedStatementTest_valuesShouldBeInsertedInTheirNamedColumns) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "hello")
      .integer(":integer", 42)
      .real(":real", 1.5)
      .boolean(":boolean", true)
      .execute();
  BOOST_REQUIRE(1 == db.countRows());
  const auto row = db.readRow(42);
  BOOST_REQUIRE("hello" == row.text);
  BOOST_REQUIRE(42 == row.integer);
  BOOST_REQUIRE(1.5 == row.real);
  BOOST_REQUIRE(1 == row.boolean);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_bindingOrderShouldNotMatter) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.boolean(":boolean", false)
      .real(":real", 2.5)
      .integer(":integer", 7)
      .text(":text", "abc")
      .execute();
  const auto row = db.readRow(7);
  BOOST_REQUIRE("abc" == row.text);
  BOOST_REQUIRE(2.5 == row.real);
  BOOST_REQUIRE(0 == row.boolean);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_textWithAQuoteShouldBeInsertedUnchanged) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "acid 'rodge'")
      .integer(":integer", 1)
      .real(":real", 0)
      .boolean(":boolean", false)
      .execute();
  BOOST_REQUIRE("acid 'rodge'" == db.readRow(1).text);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_statementShouldBeReusableForSeveralRows) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "a")
      .integer(":integer", 1)
      .real(":real", 1)
      .boolean(":boolean", true)
      .execute();
  insert.text(":text", "b")
      .integer(":integer", 2)
      .real(":real", 2)
      .boolean(":boolean", false)
      .execute();
  BOOST_REQUIRE(2 == db.countRows());
  BOOST_REQUIRE("a" == db.readRow(1).text);
  BOOST_REQUIRE("b" == db.readRow(2).text);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_unknownParameterNameShouldThrow) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  BOOST_REQUIRE_THROW(insert.text(":unknown", "a"), DatabaseException);
  // the name must have its prefix
  BOOST_REQUIRE_THROW(insert.text("text", "a"), DatabaseException);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_bindingAParameterTwiceShouldThrow) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "a");
  BOOST_REQUIRE_THROW(insert.text(":text", "b"), DatabaseException);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_executingWithAMissingParameterShouldThrow) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "a")
      .integer(":integer", 1)
      .real(":real", 1)
      .boolean(":boolean", true)
      .execute();
  // SQLite would silently reuse the :boolean of the previous row
  insert.text(":text", "b").integer(":integer", 2).real(":real", 2);
  BOOST_REQUIRE_THROW(insert.execute(), DatabaseException);
  BOOST_REQUIRE(1 == db.countRows());
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_statementShouldBeReusableAfterAMissingParameter) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "a");
  BOOST_REQUIRE_THROW(insert.execute(), DatabaseException);
  insert.text(":text", "b")
      .integer(":integer", 2)
      .real(":real", 2)
      .boolean(":boolean", true)
      .execute();
  BOOST_REQUIRE(1 == db.countRows());
  BOOST_REQUIRE("b" == db.readRow(2).text);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_constraintViolationShouldThrowAndKeepStatementReusable) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), INSERT_ROW};
  insert.text(":text", "a")
      .integer(":integer", 1)
      .real(":real", 1)
      .boolean(":boolean", true)
      .execute();
  // the column i is UNIQUE
  insert.text(":text", "b").integer(":integer", 1).real(":real", 1).boolean(":boolean", true);
  BOOST_REQUIRE_THROW(insert.execute(), DatabaseException);
  insert.text(":text", "c")
      .integer(":integer", 3)
      .real(":real", 3)
      .boolean(":boolean", true)
      .execute();
  BOOST_REQUIRE(2 == db.countRows());
  BOOST_REQUIRE("c" == db.readRow(3).text);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_statementShouldBePreparedAtTheFirstBinding) {
  const InMemoryDatabase db;
  // an existing database file can have no schema yet when the PreparedStatement is created
  PreparedStatement insert {db.get(), "INSERT INTO Later (t) VALUES (:text);"};
  db.execute("CREATE TABLE Later (t TEXT);");
  insert.text(":text", "a").execute();
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_invalidSqlShouldThrowAtTheFirstBinding) {
  const InMemoryDatabase db;
  PreparedStatement insert {db.get(), "INSERT INTO NoSuchTable (t) VALUES (:text);"};
  BOOST_REQUIRE_THROW(insert.text(":text", "a"), DatabaseException);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_selectShouldReadTheRowsMatchingTheParameters) {
  const InMemoryDatabase db;
  db.execute("INSERT INTO Row (t, i, r, b) VALUES ('a', 1, 1.5, 1), ('b', 2, 2.5, 0), "
             "('a', 3, 3.5, 0);");
  PreparedStatement select {db.get(), SELECT_ROWS};
  select.text(":text", "a");
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE(4 == select.getColumnCount());
  BOOST_REQUIRE("a" == select.getColumnAsString(0));
  BOOST_REQUIRE(1 == select.getColumnAsInt(1));
  BOOST_REQUIRE(1.5 == select.getColumnAsDouble(2));
  BOOST_REQUIRE(select.getColumnAsBool(3));
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE(3 == select.getColumnAsInt(1));
  BOOST_REQUIRE(!select.getColumnAsBool(3));
  BOOST_REQUIRE(QueryResult::NO_MORE_ROWS == select.nextRow());
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_selectWithNoMatchingRowShouldReturnNoMoreRows) {
  const InMemoryDatabase db;
  PreparedStatement select {db.get(), SELECT_ROWS};
  select.text(":text", "a");
  BOOST_REQUIRE(QueryResult::NO_MORE_ROWS == select.nextRow());
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_selectWithAQuoteInTheParameterShouldFindTheRow) {
  const InMemoryDatabase db;
  db.execute("INSERT INTO Row (t, i) VALUES ('O''Neil', 1);");
  PreparedStatement select {db.get(), SELECT_ROWS};
  select.text(":text", "O'Neil");
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE("O'Neil" == select.getColumnAsString(0));
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_selectWithAMissingParameterShouldThrow) {
  const InMemoryDatabase db;
  PreparedStatement select {db.get(), SELECT_ROWS};
  BOOST_REQUIRE_THROW(static_cast<void>(select.nextRow()), DatabaseException);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_bindingWhileReadingTheRowsShouldThrow) {
  const InMemoryDatabase db;
  db.execute("INSERT INTO Row (t, i) VALUES ('a', 1), ('a', 2);");
  PreparedStatement select {db.get(), SELECT_ROWS};
  select.text(":text", "a");
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE_THROW(select.text(":text", "b"), DatabaseException);
  BOOST_REQUIRE_THROW(select.execute(), DatabaseException);
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_selectShouldBeReusableAfterReadingAllTheRows) {
  const InMemoryDatabase db;
  db.execute("INSERT INTO Row (t, i) VALUES ('a', 1), ('b', 2);");
  PreparedStatement select {db.get(), SELECT_ROWS};
  select.text(":text", "a");
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE(QueryResult::NO_MORE_ROWS == select.nextRow());
  select.text(":text", "b");
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE(2 == select.getColumnAsInt(1));
}

BOOST_AUTO_TEST_CASE(PreparedStatementTest_readingANullTextShouldThrow) {
  const InMemoryDatabase db;
  db.execute("INSERT INTO Row (t, i) VALUES ('a', 1);");
  PreparedStatement select {db.get(), "SELECT r FROM Row WHERE t = :text;"};
  select.text(":text", "a");
  BOOST_REQUIRE(QueryResult::ONE_ROW_OR_MORE == select.nextRow());
  BOOST_REQUIRE_THROW(static_cast<void>(select.getColumnAsString(0)), DatabaseException);
}

BOOST_AUTO_TEST_SUITE_END()
