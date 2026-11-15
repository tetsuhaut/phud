#pragma once

#include <gsl/gsl> // gsl::not_null
#include <cstdint> // std::uint64_t
#include <string_view>

// forward declarations, from sqlite3.h
struct sqlite3;
struct sqlite3_stmt;

/**
 * An insertion prepared once, then executed for each row. Building then parsing a SQL text for
 * each row took most of the import time. The values are bound, so they need no escaping: a
 * player name can contain a quote.
 * Each value is bound to a named parameter of the query, in any order, then execute() inserts the
 * row:
 *   insert.text(":handId", handId).integer(":actionIndex", index).execute();
 * The texts are not copied: they must stay valid until execute() returns.
 */
class [[nodiscard]] PreparedInsert final {
private:
  gsl::not_null<sqlite3*> m_db;
  std::string_view m_sql;
  // prepared at the first insertion: an existing database file can have no schema yet
  sqlite3_stmt* m_pStatement {nullptr};
  // the bit (position - 1) is set when the parameter at this position is bound
  std::uint64_t m_boundPositions {0};

  void require(int returnCode, int expected, std::string_view what) const;
  [[noreturn]] void fail(std::string_view problem) const;
  [[nodiscard]] int bindingPosition(gsl::not_null<const char*> name);

public:
  /**
   * @param sql the query, with named parameters like :handId. It must outlive this object.
   */
  PreparedInsert(gsl::not_null<sqlite3*> db, std::string_view sql);

  PreparedInsert(const PreparedInsert&) = delete;
  PreparedInsert(PreparedInsert&&) = delete;
  PreparedInsert& operator=(const PreparedInsert&) = delete;
  PreparedInsert& operator=(PreparedInsert&&) = delete;

  // must be destroyed before the database is closed
  ~PreparedInsert();

  /**
   * Bounds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the text parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the text value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedInsert& text(gsl::not_null<const char*> name, std::string_view value);

  /**
   * Bounds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the integer parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the integer value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedInsert& integer(gsl::not_null<const char*> name, long long value);

  /**
   * Bounds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the real parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the real value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedInsert& real(gsl::not_null<const char*> name, double value);

  /**
   * Bounds a parameter value to its name in the SQL query m_sql.
   * @param name the name of the boolean parameter in the SQL query, with its prefix, e.g. ":handId"
   * @param value the boolean value to be placed in the SQL query
   * @throws DatabaseException if the query has no parameter with this name, or if it is already
   * bound for this row
   */
  PreparedInsert& boolean(gsl::not_null<const char*> name, bool value);

  /**
   * Inserts the row with the bound values, then gets ready for the next row.
   * @throws DatabaseException if not every value is bound, or if an error occurs
   */
  void execute();
}; // class PreparedInsert
