#include "db/PreparedStatement.hpp"
#include "db/Database.hpp"             // DatabaseException
#include <spdlog/fmt/bundled/format.h> // fmt::format
#include <sqlite3.h>                   // sqlite3_*
#include <bit>                         // std::popcount

namespace {
  // the bound parameters are remembered as the bits of an std::uint64_t
  constexpr int MAX_NB_PARAMETERS {64};
} // anonymous namespace

PreparedStatement::PreparedStatement(gsl::not_null<sqlite3*> db, std::string_view sql)
  : m_db {db},
    m_sql {sql} {}

PreparedStatement::~PreparedStatement() {
  sqlite3_finalize(m_pStatement);
}

void PreparedStatement::require(int returnCode, int expected, std::string_view what) const {
  if (expected != returnCode) {
    throw DatabaseException(fmt::format("Can't {} the query\n{}\nerror message:\n{}\n", what, m_sql,
                                        sqlite3_errmsg(m_db)));
  }
}

void PreparedStatement::fail(std::string_view problem) const {
  throw DatabaseException(fmt::format("{} in the query\n{}\n", problem, m_sql));
}

/**
 * @throws DatabaseException if the statement can't be prepared
 */
void PreparedStatement::prepareIfNeeded() {
  if (nullptr == m_pStatement) {
    require(sqlite3_prepare_v3(m_db, m_sql.data(), static_cast<int>(m_sql.size()),
                               SQLITE_PREPARE_PERSISTENT, &m_pStatement, nullptr),
            SQLITE_OK, "prepare");
  }
}

/**
 * Prepares the statement if needed, and marks the parameter as bound.
 * @returns the position of the parameter in the query
 * @throws DatabaseException if the statement can't be prepared, if the rows are being read, if the
 * query has no parameter with this name, or if the parameter is already bound for this execution
 */
int PreparedStatement::bindingPosition(gsl::not_null<const char*> name) {
  prepareIfNeeded();

  if (m_isReading) {
    fail(fmt::format("Can't bind the parameter '{}' while reading the rows", name.get()));
  }

  const auto position = sqlite3_bind_parameter_index(m_pStatement, name);

  if (0 == position) {
    fail(fmt::format("No parameter named '{}'", name.get()));
  }

  if (MAX_NB_PARAMETERS < position) {
    fail(fmt::format("More than {} parameters", MAX_NB_PARAMETERS));
  }

  const auto bit = std::uint64_t {1} << (position - 1);

  if (0 != (m_boundPositions & bit)) {
    fail(fmt::format("The parameter '{}' is bound twice", name.get()));
  }

  m_boundPositions |= bit;
  return position;
}

/**
 * Checks that every parameter is bound, then forgets the bindings for the next execution.
 * SQLite keeps the values of the previous execution: an unbound parameter would silently reuse it.
 * @throws DatabaseException if a parameter is not bound
 */
void PreparedStatement::requireEveryValueIsBound() {
  const auto nbBoundValues = std::popcount(m_boundPositions);
  m_boundPositions = 0;
  const auto nbParameters = sqlite3_bind_parameter_count(m_pStatement);

  if (nbParameters != nbBoundValues) {
    fail(fmt::format("Only {} of the {} parameters are bound", nbBoundValues, nbParameters));
  }
}

PreparedStatement& PreparedStatement::text(gsl::not_null<const char*> name,
                                           std::string_view value) {
  const auto position = bindingPosition(name);
  require(sqlite3_bind_text(m_pStatement, position, value.data(), static_cast<int>(value.size()),
                            SQLITE_STATIC),
          SQLITE_OK, "bind a value of");
  return *this;
}

PreparedStatement& PreparedStatement::integer(gsl::not_null<const char*> name, long long value) {
  const auto position = bindingPosition(name);
  require(sqlite3_bind_int64(m_pStatement, position, value), SQLITE_OK, "bind a value of");
  return *this;
}

PreparedStatement& PreparedStatement::real(gsl::not_null<const char*> name, double value) {
  const auto position = bindingPosition(name);
  require(sqlite3_bind_double(m_pStatement, position, value), SQLITE_OK, "bind a value of");
  return *this;
}

PreparedStatement& PreparedStatement::boolean(gsl::not_null<const char*> name, bool value) {
  // stored as 1 or 0
  return integer(name, value ? 1 : 0);
}

void PreparedStatement::execute() {
  prepareIfNeeded();

  if (m_isReading) {
    fail("Can't execute while reading the rows");
  }

  requireEveryValueIsBound();
  const auto returnCode = sqlite3_step(m_pStatement);
  // reset even after an error, so that the statement can be used again
  sqlite3_reset(m_pStatement);
  require(returnCode, SQLITE_DONE, "execute");
}

QueryResult PreparedStatement::nextRow() {
  if (!m_isReading) {
    prepareIfNeeded();
    requireEveryValueIsBound();
    m_isReading = true;
  }

  const auto returnCode = sqlite3_step(m_pStatement);

  if (SQLITE_ROW == returnCode) {
    return QueryResult::ONE_ROW_OR_MORE;
  }

  // no more rows, or an error: reset so that the statement can be used again
  m_isReading = false;
  sqlite3_reset(m_pStatement);
  require(returnCode, SQLITE_DONE, "execute");
  return QueryResult::NO_MORE_ROWS;
}

int PreparedStatement::getColumnCount() const noexcept {
  return sqlite3_column_count(m_pStatement);
}

int PreparedStatement::getColumnAsInt(int column) const noexcept {
  return sqlite3_column_int(m_pStatement, column);
}

double PreparedStatement::getColumnAsDouble(int column) const noexcept {
  return sqlite3_column_double(m_pStatement, column);
}

bool PreparedStatement::getColumnAsBool(int column) const noexcept {
  return 0 != getColumnAsInt(column);
}

std::string PreparedStatement::getColumnAsString(int column) const {
  if (const auto* text {sqlite3_column_text(m_pStatement, column)}; nullptr != text) {
    return reinterpret_cast<const char*>(text);
  }

  throw DatabaseException(
      fmt::format("Got a NULL value for the column {} of the query\n{}\n", column, m_sql));
}
