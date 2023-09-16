#pragma once

#include "containers/Vector.hpp"
#include "system/Time.hpp" // Time, uptr, String, StringView
#include "entities/Seat.hpp"
#include <array> // std::array

// forward declarations
class Action;
enum class Card : short;
enum class GameType : short;

class [[nodiscard]] Hand final {
private:
  String m_id;
  GameType m_gameType;
  String m_siteName;
  String m_tableName;
  Seat m_buttonSeat;
  Seat m_maxSeats;
  int m_level;
  long m_ante;
  Time m_date;
  std::array<Card, 5> m_heroCards;
  std::array<Card, 5> m_boardCards;
  std::array<String, 10> m_seats;
  Vector<uptr<Action>> m_actions;
  std::array<String, 10> m_winners;

public:
  struct [[nodiscard]] Params final {
    StringView id;
    GameType gameType;
    StringView siteName;
    StringView tableName;
    Seat buttonSeat;
    Seat maxSeats;
    int level;
    long ante;
    const Time& startDate;
    const std::array<String, 10>& seatPlayers;
    const std::array<Card, 5>& heroCards;
    const std::array<Card, 5>& boardCards;
    Vector<uptr<Action>> actions;
    const std::array<String, 10>& winners;
  }; // struct Params

  explicit Hand(Params& p);
  Hand(const Hand&) = delete;
  Hand(Hand&&) = delete;
  Hand& operator=(const Hand&) = delete;
  Hand& operator=(Hand&&) = delete;
  ~Hand();
  [[nodiscard]] Vector<const Action*> viewActions() const { return mkView(m_actions); }
  [[nodiscard]] String getId() const noexcept { return m_id; }
  [[nodiscard]] GameType getGameType() const noexcept { return m_gameType; }
  [[nodiscard]] String getSiteName() const noexcept { return m_siteName; }
  [[nodiscard]] String getTableName() const noexcept { return m_tableName; }
  [[nodiscard]] std::array<String, 10> getSeats() const noexcept { return m_seats; }
  [[nodiscard]] Seat getButtonSeat() const noexcept { return m_buttonSeat; }
  [[nodiscard]] Seat getMaxSeats() const noexcept { return m_maxSeats; }
  [[nodiscard]] int getLevel()const noexcept { return m_level; }
  [[nodiscard]] long getAnte() const noexcept { return m_ante; }
  [[nodiscard]] Time getStartDate() const noexcept { return m_date; }
  [[nodiscard]] bool isPlayerInvolved(StringView name) const;
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
  [[nodiscard]] bool isWinner(StringView playerName) const noexcept;
}; // class Hand
