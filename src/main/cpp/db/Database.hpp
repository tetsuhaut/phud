#pragma once

#include "containers/Span.hpp"
#include "containers/Vector.hpp"
#include "entities/Seat.hpp"
#include "language/PhudException.hpp" // StringView
#include "strings/String.hpp"
#include "system/memory.hpp" // uptr

// forward declarations
class CashGame;
class Player;
class PlayerStatistics;
class Site;
struct TableStatistics;
class Tournament;

/**
 * The database where each entity is persisted.
 */
class [[nodiscard]] Database final {
private:
  struct Implementation;
  uptr<Implementation> m_pImpl;

public:
  /**
   * Creates a data access layer object, in memory: no file is created.
   * The database will be deleted when the created object is destroyed.
   */
  Database();

  /**
  * Creates a data access layer object, using the given file.
  * @param name the database file name. If it is an empty file, the database and its schema will be created.
  * @throws DatabaseException in case of problem getting the query SQL code or opeing the database
  * file.
  */
  explicit Database(StringView name);

  // if we define a default constructor and destructor, we should define all of the default operations
  Database(const Database&) = delete;
  Database(Database&&) = delete;
  Database& operator=(const Database&) = delete;
  Database& operator=(Database&&) = delete;

  ~Database();
  void save(const Site& site);
  void save(const CashGame& game);
  void save(const Tournament& game);
  void save(Span<const Player* const> players);
  // exported for unit tests
  [[nodiscard]] uptr<PlayerStatistics> readPlayerStatistics(StringView sn,
      StringView playerName) const;
  /**
   * Retrieves the stats for each player of a given table.
   */
  struct [[nodiscard]] ReadTableStatisticsArgs final { StringView site; StringView table; };
  [[nodiscard]] TableStatistics readTableStatistics(const ReadTableStatisticsArgs& args) const;
  [[nodiscard]] String getDbName() const noexcept;
  [[nodiscard]] bool isInMemory() const noexcept;

  // for unit testing
  [[nodiscard]] Seat getTableMaxSeat(StringView site, StringView table) const;
}; // class Database

class [[nodiscard]] DatabaseException final : public PhudException {
public:
  using PhudException::PhudException;
};
