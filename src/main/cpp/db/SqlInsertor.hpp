#pragma once

#include "entities/Seat.hpp"
#include "strings/String.hpp" // StringView, phud::strings

enum class ActionType : short;
enum class Card : short;
enum class Limit : short;
enum class Street : short;
enum class Variant : short;

/**
 * Builds INSERT SQL queries, possibly with multiple value sets.
 * Example:
 * INSERT INTO someTable (column1, column2, column3) VALUES (1, 2, 3), (4, 5, 6);
 */
class [[nodiscard]] SqlInsertor final {
private:
  String m_query;
  String m_valueModel;
  String m_values;

public:
  /**
   * @param sqlTemplate an SQL query model, such as
   *        INSERT INTO someTable (column1, column2, ...) VALUES (?, '?', ...);
   */
  explicit SqlInsertor(StringView sqlTemplate);
  SqlInsertor(const SqlInsertor&) = delete;
  SqlInsertor(SqlInsertor&&) = delete;
  SqlInsertor& operator=(const SqlInsertor&) = delete;
  SqlInsertor& operator=(SqlInsertor&&) = delete;
  ~SqlInsertor() = default;

  /**
   * @returns a SQL INSERT query.
   */
  [[nodiscard]] String build();


  /*[[nodiscard]]*/ SqlInsertor& newInsert();
  [[nodiscard]] SqlInsertor& siteName(StringView value);
  [[nodiscard]] SqlInsertor& isHero(bool value);
  [[nodiscard]] SqlInsertor& comments(StringView value);
  [[nodiscard]] SqlInsertor& playerSeat(Seat value);
  [[nodiscard]] SqlInsertor& playerName(StringView value);
  [[nodiscard]] SqlInsertor& isWinner(bool value);
  [[nodiscard]] SqlInsertor& tableName(StringView value);
  [[nodiscard]] SqlInsertor& buttonSeat(Seat value);
  [[nodiscard]] SqlInsertor& maxSeats(Seat value);
  [[nodiscard]] SqlInsertor& level(int value);
  [[nodiscard]] SqlInsertor& ante(int value);
  [[nodiscard]] SqlInsertor& heroCard1(Card value);
  [[nodiscard]] SqlInsertor& heroCard2(Card value);
  [[nodiscard]] SqlInsertor& heroCard3(Card value);
  [[nodiscard]] SqlInsertor& heroCard4(Card value);
  [[nodiscard]] SqlInsertor& heroCard5(Card value);
  [[nodiscard]] SqlInsertor& boardCard1(Card value);
  [[nodiscard]] SqlInsertor& boardCard2(Card value);
  [[nodiscard]] SqlInsertor& boardCard3(Card value);
  [[nodiscard]] SqlInsertor& boardCard4(Card value);
  [[nodiscard]] SqlInsertor& boardCard5(Card value);
  [[nodiscard]] SqlInsertor& gameId(StringView value);
  [[nodiscard]] SqlInsertor& gameName(StringView value);
  [[nodiscard]] SqlInsertor& handId(StringView value);
  [[nodiscard]] SqlInsertor& street(Street value);
  [[nodiscard]] SqlInsertor& actionType(ActionType value);
  [[nodiscard]] SqlInsertor& actionIndex(std::size_t value);
  [[nodiscard]] SqlInsertor& betAmount(double value);
  [[nodiscard]] SqlInsertor& variant(Variant value);
  [[nodiscard]] SqlInsertor& limitType(Limit value);
  [[nodiscard]] SqlInsertor& isRealMoney(bool value);
  [[nodiscard]] SqlInsertor& nbMaxSeats(Seat value);
  [[nodiscard]] SqlInsertor& startDate(StringView value);
  [[nodiscard]] SqlInsertor& tournamentId(StringView value);
  [[nodiscard]] SqlInsertor& buyIn(double value);
  [[nodiscard]] SqlInsertor& cashGameId(StringView value);
  [[nodiscard]] SqlInsertor& smallBlind(double value);
  [[nodiscard]] SqlInsertor& bigBlind(double value);
}; // class SqlInsertor

[[nodiscard]] String formatSQL(StringView s);