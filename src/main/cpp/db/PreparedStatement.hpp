#pragma once

#include <gsl/gsl> // gsl::not_null
#include <cstdint> // std::uint8_t, std::uint64_t
#include <string>
#include <string_view>

// forward declarations, from sqlite3.h
struct sqlite3;
struct sqlite3_stmt;

enum class /*[[nodiscard]]*/ QueryResult : std::uint8_t { NO_MORE_ROWS, ONE_ROW_OR_MORE };

/**
 * A SQL query prepared once, then executed as many times as needed. The values are bound, so they
 * need no escaping: a player name can contain a quote.
 * Each value is bound to a named parameter of the query, in any order. Then:
 * - for an insertion, execute() inserts the row, and the statement is ready for the next one.
 *   Building then parsing a SQL text for each row took most of the import time.
 *     insert.text(":handId", handId).integer(":actionIndex", index).execute();
 * - for a selection, nextRow() steps to each row of the result, read with the getColumnAs*().
 *     select.text(":siteName", site);
 *     while (QueryResult::ONE_ROW_OR_MORE == select.nextRow()) { select.getColumnAsInt(0); }
 * The texts are not copied: they must stay valid until execute() returns, or until the last row
 * is read.
 */
class [[nodiscard]] PreparedStatement final {
private:
  gsl::not_null<sqlite3*> m_db;
  std::string_view m_sql;
  // prepared at the first use: an existing database file can have no schema yet
  sqlite3_stmt* m_pStatement {nullptr};
  // the bit (position - 1) is set when the parameter at this position is bound
  std::uint64_t m_boundPositions {0};
  // true between the first nextRow() and the end of the result
  bool m_isReading {false};

  void require(int returnCode, int expected, std::string_view what) const;
  [[noreturn]] void fail(std::string_view problem) const;
  void prepareIfNeeded();
  [[nodiscard]] int bindingPosition(gsl::not_null<const char*> name);
  void requireEveryValueIsBound();

public:
  /**
   * @param sql the query, with named parameters like :handId. It must outlive this object.
   */
  PreparedStatement(gsl::not_null<sqlite3*> db, std::string_view sql);
  ~PreparedStatement(); // must be destroyed before the database is closed

  PreparedStatement(const PreparedStatement&) = delete;
  PreparedStatement(PreparedStatement&&) = delete;
  PreparedStatement& operator=(const PreparedStatement&) = delete;
  PreparedStatement& operator=(PreparedStatement&&) = delete;

  /**
   * Binds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the text parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the text value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedStatement& text(gsl::not_null<const char*> name, std::string_view value);

  /**
   * Binds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the integer parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the integer value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedStatement& integer(gsl::not_null<const char*> name, long long value);

  /**
   * Binds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the real parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the real value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedStatement& real(gsl::not_null<const char*> name, double value);

  /**
   * Binds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the boolean parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the boolean value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedStatement& boolean(gsl::not_null<const char*> name, bool value);

  /**
   * Executes a query returning no row, like an insertion, with the bound values, then gets ready
   * for the next execution.
   * @throws DatabaseException if not every value is bound, if the query returns a row, or if an
   * error occurs
   */
  void execute();

  /**
   * Steps to the next row of the result of a selection. The first call executes the query with
   * the bound values. Once there are no more rows, the statement is ready for the next execution.
   * @returns QueryResult::ONE_ROW_OR_MORE when a row can be read with the getColumnAs*(), else
   * QueryResult::NO_MORE_ROWS
   * @throws DatabaseException if not every value is bound, or if an error occurs
   */
  [[nodiscard]] QueryResult nextRow();

  /**
   * The getColumn*() read the current row, after nextRow() returned
   * QueryResult::ONE_ROW_OR_MORE.
   * @param column the index of the column in the result, starting at 0
   */
  [[nodiscard]] int getColumnCount() const noexcept;
  [[nodiscard]] int getColumnAsInt(int column) const noexcept;
  [[nodiscard]] double getColumnAsDouble(int column) const noexcept;
  [[nodiscard]] bool getColumnAsBool(int column) const noexcept;

  /**
   * @throws DatabaseException if the value is NULL
   */
  [[nodiscard]] std::string getColumnAsString(int column) const;
}; // class PreparedStatement
