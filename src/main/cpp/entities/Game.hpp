#pragma once

#include "system/Time.hpp" // std::string, std::string_view, Time

#include <vector>

// forward declarations
class Hand;
enum class Seat : short;

/**
 * Game variants that phud understands.
 */
enum class /*[[nodiscard]]*/ Variant : short {
  none, holdem, omaha, omaha5
};

/**
 * Game limits that phud understands.
 */
enum class /*[[nodiscard]]*/ Limit : short {
  none, noLimit, potLimit
};

/**
 * A game.
 */
class [[nodiscard]] Game final {
private:
  // Memory layout optimized: largest to smallest to minimize padding
  std::string m_id;
  std::string m_site;
  std::string m_name;
  std::vector<std::unique_ptr<Hand>> m_hands;
  Time m_startDate;
  Variant m_variant;
  Limit m_limitType;
  Seat m_nbMaxSeats;
  bool m_isRealMoney;

public:

  struct [[nodiscard]] Params final {
    std::string_view id;
    std::string_view siteName;
    std::string_view gameName;
    Variant variant;
    Limit limitType;
    bool isRealMoney;
    Seat nbMaxSeats;
    const Time& startDate;
  };

  explicit Game(const Params& args);
  Game(const Game&) = delete;
  Game(Game&&) = delete;
  Game& operator=(const Game&) = delete;
  Game& operator=(Game&&) = delete;
  ~Game();

  void addHand(std::unique_ptr<Hand> hand);
  [[nodiscard]] constexpr const std::string& getName() const noexcept { return m_name; }
  [[nodiscard]] constexpr bool isRealMoney() const noexcept { return m_isRealMoney; }
  [[nodiscard]] Time getStartDate() const noexcept { return m_startDate; }
  [[nodiscard]] std::vector<const Hand*> viewHands() const;
  [[nodiscard]] std::vector<const Hand*> viewHands(std::string_view player) const;
  [[nodiscard]] constexpr const std::string& getSiteName() const noexcept { return m_site; }
  [[nodiscard]] constexpr const std::string& getId() const noexcept { return m_id; }
  [[nodiscard]] constexpr Variant getVariant() const noexcept { return m_variant; }
  [[nodiscard]] constexpr Limit getLimitType() const noexcept { return m_limitType; }
  [[nodiscard]] constexpr Seat getMaxNbSeats() const noexcept { return m_nbMaxSeats; }
}; // class Game

class [[nodiscard]] Tournament final {
private:
  // Memory layout optimized: largest to smallest to minimize padding
  std::unique_ptr<Game> m_game;
  double m_buyIn;

public:

  struct [[nodiscard]] Params final {
    std::string_view id;
    std::string_view siteName;
    std::string_view tournamentName;
    Variant variant;
    Limit limit;
    bool isRealMoney;
    Seat nbMaxSeats;
    double buyIn;
    const Time& startDate;
  };

  explicit Tournament(const Params& p);
  Tournament(const Tournament&) = delete;
  Tournament(Tournament&&) = delete;
  Tournament& operator=(const Tournament&) = delete;
  Tournament& operator=(Tournament&&) = delete;
  ~Tournament();
  void addHand(std::unique_ptr<Hand> hand) const;
  [[nodiscard]] constexpr const std::string& getName() const noexcept { return m_game->getName(); }
  [[nodiscard]] constexpr bool isRealMoney() const noexcept { return m_game->isRealMoney(); }
  [[nodiscard]] Time getStartDate() const noexcept { return m_game->getStartDate(); }
  [[nodiscard]] std::vector<const Hand*> viewHands() const { return m_game->viewHands(); }
  [[nodiscard]] std::vector<const Hand*> viewHands(std::string_view player) const { return m_game->viewHands(player); }
  [[nodiscard]] constexpr const std::string& getSiteName() const noexcept { return m_game->getSiteName(); }
  [[nodiscard]] constexpr const std::string& getId() const noexcept { return m_game->getId(); }
  [[nodiscard]] constexpr Variant getVariant() const noexcept { return m_game->getVariant(); }
  [[nodiscard]] constexpr Limit getLimitType() const noexcept { return m_game->getLimitType(); }
  [[nodiscard]] constexpr Seat getMaxNbSeats() const noexcept { return m_game->getMaxNbSeats(); }
  [[nodiscard]] constexpr double getBuyIn() const noexcept { return m_buyIn; }
}; // class Tournament

class [[nodiscard]] CashGame final {
private:
  // Memory layout optimized: largest to smallest to minimize padding
  std::unique_ptr<Game> m_game;
  double m_smallBlind;
  double m_bigBlind;

public:

  struct [[nodiscard]] Params final {
    std::string_view id;
    std::string_view siteName;
    std::string_view cashGameName;
    Variant variant;
    Limit limit;
    bool isRealMoney;
    Seat nbMaxSeats;
    double smallBlind;
    double bigBlind;
    const Time& startDate;
  };

  explicit CashGame(const Params& p);
  CashGame(const CashGame&) = delete;
  CashGame(CashGame&&) = delete;
  CashGame& operator=(const CashGame&) = delete;
  CashGame& operator=(CashGame&&) = delete;
  ~CashGame();
  void addHand(std::unique_ptr<Hand> hand) const;
  [[nodiscard]] constexpr const std::string& getName() const noexcept { return m_game->getName(); }
  [[nodiscard]] constexpr bool isRealMoney() const noexcept { return m_game->isRealMoney(); }
  [[nodiscard]] Time getStartDate() const noexcept { return m_game->getStartDate(); }
  [[nodiscard]] std::vector<const Hand*> viewHands() const { return m_game->viewHands(); }
  [[nodiscard]] std::vector<const Hand*> viewHands(std::string_view player) const { return m_game->viewHands(player); }
  [[nodiscard]] constexpr const std::string& getSiteName() const noexcept { return m_game->getSiteName(); }
  [[nodiscard]] constexpr const std::string& getId() const noexcept { return m_game->getId(); }
  [[nodiscard]] constexpr Variant getVariant() const noexcept { return m_game->getVariant(); }
  [[nodiscard]] constexpr Limit getLimitType() const noexcept { return m_game->getLimitType(); }
  [[nodiscard]] constexpr Seat getMaxNbSeats() const noexcept { return m_game->getMaxNbSeats(); }
  [[nodiscard]] constexpr double getSmallBlind() const noexcept { return m_smallBlind; }
  [[nodiscard]] constexpr double getBigBlind() const noexcept { return m_bigBlind; }
}; // class CashGame

// exported methods
[[nodiscard]] std::string_view toString(Variant variant);
[[nodiscard]] std::string_view toString(Limit limitype);
