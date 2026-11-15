#include "db/PreparedInsert.hpp"
#include "db/Database.hpp"                   // DatabaseException
#include <spdlog/fmt/bundled/format.h>       // fmt::format
#include <sqlite3.h>                         // sqlite3_*
#include <bit>                               // std::popcount

namespace {
  // the bound parameters are remembered as the bits of an std::uint64_t
  constexpr int MAX_NB_PARAMETERS {64};
} // anonymous namespace

PreparedInsert::PreparedInsert(gsl::not_null<sqlite3*> db, std::string_view sql)
  : m_db {db},
    m_sql {sql} {}

PreparedInsert::~PreparedInsert() { sqlite3_finalize(m_pStatement); }

void PreparedInsert::require(int returnCode, int expected, std::string_view what) const {
  if (expected != returnCode) {
    throw DatabaseException(fmt::format("Can't {} the query\n{}\nerror message:\n{}\n", what,
                                        m_sql, sqlite3_errmsg(m_db)));
  }
}

void PreparedInsert::fail(std::string_view problem) const {
  throw DatabaseException(fmt::format("{} in the query\n{}\n", problem, m_sql));
}

/**
 * Prepares the statement if needed, and marks the parameter as bound.
 * @returns the position of the parameter in the query
 * @throws DatabaseException if the statement can't be prepared, if the query has no parameter
 * with this name, or if the parameter is already bound for this row
 */
int PreparedInsert::bindingPosition(gsl::not_null<const char*> name) {
  if (nullptr == m_pStatement) {
    require(sqlite3_prepare_v3(m_db, m_sql.data(), static_cast<int>(m_sql.size()),
                               SQLITE_PREPARE_PERSISTENT, &m_pStatement, nullptr),
            SQLITE_OK, "prepare");
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

PreparedInsert& PreparedInsert::text(gsl::not_null<const char*> name, std::string_view value) {
  const auto position = bindingPosition(name);
  require(sqlite3_bind_text(m_pStatement, position, value.data(), static_cast<int>(value.size()),
                            SQLITE_STATIC),
          SQLITE_OK, "bind a value of");
  return *this;
}

PreparedInsert& PreparedInsert::integer(gsl::not_null<const char*> name, long long value) {
  const auto position = bindingPosition(name);
  require(sqlite3_bind_int64(m_pStatement, position, value), SQLITE_OK, "bind a value of");
  return *this;
}

PreparedInsert& PreparedInsert::real(gsl::not_null<const char*> name, double value) {
  const auto position = bindingPosition(name);
  require(sqlite3_bind_double(m_pStatement, position, value), SQLITE_OK, "bind a value of");
  return *this;
}

PreparedInsert& PreparedInsert::boolean(gsl::not_null<const char*> name, bool value) {
  // stored as 1 or 0
  return integer(name, value ? 1 : 0);
}

void PreparedInsert::execute() {
  // SQLite keeps the values of the previous row: an unbound parameter would silently reuse it
  const auto nbBoundValues = std::popcount(m_boundPositions);
  m_boundPositions = 0;
  require(nbBoundValues, sqlite3_bind_parameter_count(m_pStatement), "bind every value of");
  const auto returnCode = sqlite3_step(m_pStatement);
  // reset even after an error, so that the statement can be used again
  sqlite3_reset(m_pStatement);
  require(returnCode, SQLITE_DONE, "execute");
}
