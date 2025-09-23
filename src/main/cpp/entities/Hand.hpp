#pragma once

#include "constants/TableConstants.hpp"
#include "system/Time.hpp" // Time, std::unique_ptr, std::string, std::string_view
#include <array>
#include <vector>

// forward declarations
class Action;
enum class Card : short;
enum class GameType : short;
enum class Seat : short;

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
  std::array<Card, TableConstants::MAX_CARDS> m_heroCards;
  std::array<Card, TableConstants::MAX_CARDS> m_boardCards;
  std::array<std::string, TableConstants::MAX_SEATS> m_seats;
  std::vector<std::unique_ptr<Action>> m_actions;
  std::array<std::string, TableConstants::MAX_SEATS> m_winners;

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
    const std::array<std::string, TableConstants::MAX_SEATS>& seatPlayers;
    const std::array<Card, TableConstants::MAX_CARDS>& heroCards;
    const std::array<Card, TableConstants::MAX_CARDS>& boardCards;
    std::vector<std::unique_ptr<Action>> actions;
    const std::array<std::string, TableConstants::MAX_SEATS>& winners;
  }; // struct Params

  explicit Hand(Params& p);
  Hand(const Hand&) = delete;
  Hand(Hand&&) = delete;
  Hand& operator=(const Hand&) = delete;
  Hand& operator=(Hand&&) = delete;
  ~Hand();
  [[nodiscard]] std::vector<const Action*> viewActions() const;
  [[nodiscard]] constexpr const std::string& getId() const noexcept { return m_id; }
  [[nodiscard]] constexpr GameType getGameType() const noexcept { return m_gameType; }
  [[nodiscard]] constexpr const std::string& getSiteName() const noexcept { return m_siteName; }
  [[nodiscard]] constexpr const std::string& getTableName() const noexcept { return m_tableName; }
  [[nodiscard]] constexpr const std::array<std::string, TableConstants::MAX_SEATS>& getSeats() const noexcept { return m_seats; }
  [[nodiscard]] constexpr Seat getButtonSeat() const noexcept { return m_buttonSeat; }
  [[nodiscard]] constexpr Seat getMaxSeats() const noexcept { return m_maxSeats; }
  [[nodiscard]] constexpr int getLevel()const noexcept { return m_level; }
  [[nodiscard]] constexpr long getAnte() const noexcept { return m_ante; }
  [[nodiscard]] constexpr Time getStartDate() const noexcept { return m_date; }
  [[nodiscard]] bool isPlayerInvolved(std::string_view name) const;
  [[nodiscard]] constexpr Card getHeroCard1() const { return m_heroCards.at(0); }
  [[nodiscard]] constexpr Card getHeroCard2() const { return m_heroCards.at(1); }
  [[nodiscard]] constexpr Card getHeroCard3() const { return m_heroCards.at(2); }
  [[nodiscard]] constexpr Card getHeroCard4() const { return m_heroCards.at(3); }
  [[nodiscard]] constexpr Card getHeroCard5() const { return m_heroCards.at(4); }
  [[nodiscard]] constexpr Card getBoardCard1() const { return m_boardCards.at(0); }
  [[nodiscard]] constexpr Card getBoardCard2() const { return m_boardCards.at(1); }
  [[nodiscard]] constexpr Card getBoardCard3() const { return m_boardCards.at(2); }
  [[nodiscard]] constexpr Card getBoardCard4() const { return m_boardCards.at(3); }
  [[nodiscard]] constexpr Card getBoardCard5() const { return m_boardCards.at(4); }
  [[nodiscard]] bool isWinner(std::string_view playerName) const noexcept;
}; // class Hand
