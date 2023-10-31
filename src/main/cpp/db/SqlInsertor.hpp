#pragma once

#include "entities/Seat.hpp"

#include <string>
#include <string_view>

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
  std::string m_query;
  std::string m_valueModel;
  std::string m_values;

public:
  /**
   * @param sqlTemplate an SQL query model, such as
   *        INSERT INTO someTable (column1, column2, ...) VALUES (?, '?', ...);
   */
  explicit SqlInsertor(std::string_view sqlTemplate);
  SqlInsertor(const SqlInsertor&) = delete;
  SqlInsertor(SqlInsertor&&) = delete;
  SqlInsertor& operator=(const SqlInsertor&) = delete;
  SqlInsertor& operator=(SqlInsertor&&) = delete;
  ~SqlInsertor() = default;

  /**
   * @returns a SQL INSERT query.
   */
  [[nodiscard]] std::string build();


  /*[[nodiscard]]*/ SqlInsertor& newInsert();
  [[nodiscard]] SqlInsertor& siteName(std::string_view value);
  [[nodiscard]] SqlInsertor& isHero(bool value);
  [[nodiscard]] SqlInsertor& comments(std::string_view value);
  [[nodiscard]] SqlInsertor& playerSeat(Seat value);
  [[nodiscard]] SqlInsertor& playerName(std::string_view value);
  [[nodiscard]] SqlInsertor& isWinner(bool value);
  [[nodiscard]] SqlInsertor& tableName(std::string_view value);
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
  [[nodiscard]] SqlInsertor& gameId(std::string_view value);
  [[nodiscard]] SqlInsertor& gameName(std::string_view value);
  [[nodiscard]] SqlInsertor& handId(std::string_view value);
  [[nodiscard]] SqlInsertor& street(Street value);
  [[nodiscard]] SqlInsertor& actionType(ActionType value);
  [[nodiscard]] SqlInsertor& actionIndex(std::size_t value);
  [[nodiscard]] SqlInsertor& betAmount(double value);
  [[nodiscard]] SqlInsertor& variant(Variant value);
  [[nodiscard]] SqlInsertor& limitType(Limit value);
  [[nodiscard]] SqlInsertor& isRealMoney(bool value);
  [[nodiscard]] SqlInsertor& nbMaxSeats(Seat value);
  [[nodiscard]] SqlInsertor& startDate(std::string_view value);
  [[nodiscard]] SqlInsertor& tournamentId(std::string_view value);
  [[nodiscard]] SqlInsertor& buyIn(double value);
  [[nodiscard]] SqlInsertor& cashGameId(std::string_view value);
  [[nodiscard]] SqlInsertor& smallBlind(double value);
  [[nodiscard]] SqlInsertor& bigBlind(double value);
}; // class SqlInsertor

[[nodiscard]] std::string formatSQL(std::string_view s);