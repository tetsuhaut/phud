#pragma once

#include "containers/Vector.hpp" // uptr
#include "entities/Seat.hpp"
#include "system/Time.hpp" // String, StringView, Time

// forward declarations
class Hand;

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
  String m_id;
  String m_site;
  String m_name;
  Variant m_variant;
  Limit m_limitType;
  bool m_isRealMoney;
  Seat m_nbMaxSeats;
  Time m_startDate;
  Vector<uptr<Hand>> m_hands;

public:

  struct [[nodiscard]] Params final {
    StringView id;
    StringView siteName;
    StringView gameName;
    Variant variant;
    Limit limitType;
    bool isRealMoney;
    Seat nbMaxSeats;
    const Time& startDate;
  };

  Game(const Params& args);
  Game(const Game&) = delete;
  Game(Game&&) = delete;
  Game& operator=(const Game&) = delete;
  Game& operator=(Game&&) = delete;
  ~Game();

  void addHand(uptr<Hand> hand);
  // gcc 10.2 can't do constexpr std::string
  [[nodiscard]] /*constexpr*/ String getName() const noexcept { return m_name; }
  [[nodiscard]] constexpr bool isRealMoney() const noexcept { return m_isRealMoney; }
  [[nodiscard]] Time getStartDate() const noexcept { return m_startDate; }
  [[nodiscard]] Vector<const Hand*> viewHands() const;
  [[nodiscard]] Vector<const Hand*> viewHands(StringView player) const;
  [[nodiscard]] /*constexpr*/ String getSiteName() const noexcept { return m_site; }
  [[nodiscard]] /*constexpr*/ String getId() const noexcept { return m_id; }
  [[nodiscard]] constexpr Variant getVariant() const noexcept { return m_variant; }
  [[nodiscard]] constexpr Limit getLimitType() const noexcept { return m_limitType; }
  [[nodiscard]] constexpr Seat getMaxNbSeats() const noexcept { return m_nbMaxSeats; }
}; // class Game

class [[nodiscard]] Tournament final {
private:
  double m_buyIn;
  uptr<Game> m_game;

public:

  struct [[nodiscard]] Params final {
    StringView id;
    StringView siteName;
    StringView tournamentName;
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
  void addHand(uptr<Hand> hand);
  // gcc 10.2 can't do constexpr std::string
  [[nodiscard]] /*constexpr*/ String getName() const noexcept { return m_game->getName(); }
  [[nodiscard]] /*constexpr*/ bool isRealMoney() const noexcept { return m_game->isRealMoney(); }
  [[nodiscard]] Time getStartDate() const noexcept { return m_game->getStartDate(); }
  [[nodiscard]] Vector<const Hand*> viewHands() const { return m_game->viewHands(); }
  [[nodiscard]] Vector<const Hand*> viewHands(StringView player) const { return m_game->viewHands(player); }
  [[nodiscard]] /*constexpr*/ String getSiteName() const noexcept { return m_game->getSiteName(); }
  [[nodiscard]] /*constexpr*/ String getId() const noexcept { return m_game->getId(); }
  [[nodiscard]] /*constexpr*/ Variant getVariant() const noexcept { return m_game->getVariant(); }
  [[nodiscard]] /*constexpr*/ Limit getLimitType() const noexcept { return m_game->getLimitType(); }
  [[nodiscard]] /*constexpr*/ Seat getMaxNbSeats() const noexcept { return m_game->getMaxNbSeats(); }
  [[nodiscard]] constexpr double getBuyIn() const noexcept { return m_buyIn; }
}; // class Tournament

class [[nodiscard]] CashGame final {
private:
  double m_smallBlind;
  double m_bigBlind;
  uptr<Game> m_game;

public:

  struct [[nodiscard]] Params final {
    StringView id;
    StringView siteName;
    StringView cashGameName;
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
  void addHand(uptr<Hand> hand);
  // gcc 10.2 can't do constexpr std::string
  [[nodiscard]] /*constexpr*/ String getName() const noexcept { return m_game->getName(); }
  [[nodiscard]] /*constexpr*/ bool isRealMoney() const noexcept { return m_game->isRealMoney(); }
  [[nodiscard]] Time getStartDate() const noexcept { return m_game->getStartDate(); }
  [[nodiscard]] Vector<const Hand*> viewHands() const { return m_game->viewHands(); }
  [[nodiscard]] Vector<const Hand*> viewHands(StringView player) const { return m_game->viewHands(player); }
  [[nodiscard]] /*constexpr*/ String getSiteName() const noexcept { return m_game->getSiteName(); }
  [[nodiscard]] /*constexpr*/ String getId() const noexcept { return m_game->getId(); }
  [[nodiscard]] /*constexpr*/ Variant getVariant() const noexcept { return m_game->getVariant(); }
  [[nodiscard]] /*constexpr*/ Limit getLimitType() const noexcept { return m_game->getLimitType(); }
  [[nodiscard]] /*constexpr*/ Seat getMaxNbSeats() const noexcept { return m_game->getMaxNbSeats(); }
  [[nodiscard]] constexpr double getSmallBlind() const noexcept { return m_smallBlind; }
  [[nodiscard]] constexpr double getBigBlind() const noexcept { return m_bigBlind; }
}; // class CashGame

// exported methods
[[nodiscard]] StringView toString(Variant variant);
[[nodiscard]] StringView toString(Limit limitype);
