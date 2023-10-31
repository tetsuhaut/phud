#pragma once

#include "system/Time.hpp" // Time, std::unique_ptr, std::string, std::string_view
#include "entities/Seat.hpp"

#include <array>
#include <vector>

// forward declarations
class Action;
enum class Card : short;
enum class GameType : short;

class [[nodiscard]] Hand final {
private:
  std::string m_id;
  GameType m_gameType;
  std::string m_siteName;
  std::string m_tableName;
  Seat m_buttonSeat;
  Seat m_maxSeats;
  int m_level;
  long m_ante;
  Time m_date;
  std::array<Card, 5> m_heroCards;
  std::array<Card, 5> m_boardCards;
  std::array<std::string, 10> m_seats;
  std::vector<std::unique_ptr<Action>> m_actions;
  std::array<std::string, 10> m_winners;

public:
  struct [[nodiscard]] Params final {
    std::string_view id;
    GameType gameType;
    std::string_view siteName;
    std::string_view tableName;
    Seat buttonSeat;
    Seat maxSeats;
    int level;
    long ante;
    const Time& startDate;
    const std::array<std::string, 10>& seatPlayers;
    const std::array<Card, 5>& heroCards;
    const std::array<Card, 5>& boardCards;
    std::vector<std::unique_ptr<Action>> actions;
    const std::array<std::string, 10>& winners;
  }; // struct Params

  explicit Hand(Params& p);
  Hand(const Hand&) = delete;
  Hand(Hand&&) = delete;
  Hand& operator=(const Hand&) = delete;
  Hand& operator=(Hand&&) = delete;
  ~Hand();
  [[nodiscard]] std::vector<const Action*> viewActions() const;
  [[nodiscard]] std::string getId() const noexcept { return m_id; }
  [[nodiscard]] GameType getGameType() const noexcept { return m_gameType; }
  [[nodiscard]] std::string getSiteName() const noexcept { return m_siteName; }
  [[nodiscard]] std::string getTableName() const noexcept { return m_tableName; }
  [[nodiscard]] std::array<std::string, 10> getSeats() const noexcept { return m_seats; }
  [[nodiscard]] Seat getButtonSeat() const noexcept { return m_buttonSeat; }
  [[nodiscard]] Seat getMaxSeats() const noexcept { return m_maxSeats; }
  [[nodiscard]] int getLevel()const noexcept { return m_level; }
  [[nodiscard]] long getAnte() const noexcept { return m_ante; }
  [[nodiscard]] Time getStartDate() const noexcept { return m_date; }
  [[nodiscard]] bool isPlayerInvolved(std::string_view name) const;
  [[nodiscard]] Card getHeroCard1() const { return m_heroCards.at(0); }
  [[nodiscard]] Card getHeroCard2() const { return m_heroCards.at(1); }
  [[nodiscard]] Card getHeroCard3() const { return m_heroCards.at(2); }
  [[nodiscard]] Card getHeroCard4() const { return m_heroCards.at(3); }
  [[nodiscard]] Card getHeroCard5() const { return m_heroCards.at(4); }
  [[nodiscard]] Card getBoardCard1() const { return m_boardCards.at(0); }
  [[nodiscard]] Card getBoardCard2() const { return m_boardCards.at(1); }
  [[nodiscard]] Card getBoardCard3() const { return m_boardCards.at(2); }
  [[nodiscard]] Card getBoardCard4() const { return m_boardCards.at(3); }
  [[nodiscard]] Card getBoardCard5() const { return m_boardCards.at(4); }
  [[nodiscard]] bool isWinner(std::string_view playerName) const noexcept;
}; // class Hand
