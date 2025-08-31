#include "entities/Action.hpp" // ps::toAmount
#include "entities/Card.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp" // Time
#include "entities/Player.hpp"
#include "entities/Seat.hpp"
#include "filesystem/TextFile.hpp"
#include "history/GameData.hpp"
#include "history/PokerSiteHandBuilder.hpp" // split, parseSeats
#include "history/WinamaxHandBuilder.hpp" // Pair
#include "language/limits.hpp" // toInt
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/ProgramInfos.hpp"
#include "threads/PlayerCache.hpp"

#include <optional>

static Logger LOG { CURRENT_FILE_NAME };

namespace ps = phud::strings;

constexpr static std::array FIVE_NONE_CARDS { Card::none, Card::none, Card::none, Card::none, Card::none };

[[nodiscard]] static std::array<Card, 5> parseCards(std::string_view line) {
  const auto pos { line.rfind('[') + 1 };
  const auto strCards { line.substr(pos, line.rfind(']') - pos) };
  const auto& cardsStr { split(strCards, " ") };
  std::array<Card, 5> ret;
  std::transform(cardsStr.begin(), cardsStr.end(), ret.begin(), toCard);
  return ret;
}

static constexpr auto MINUS_LENGTH { ps::length(" - ") }; // nb char without '\0
static constexpr auto HAND_ID_LENGTH { ps::length(" - HandId: #") }; // nb char without '\0

struct [[nodiscard]] StartOfWinamaxPokerLine final {
  std::size_t m_handIdPos;
  Time m_handStartDate;
  std::string m_handId;
};

// Returns the HandId position, the Hand start time and the Hand Id
[[nodiscard]] static inline StartOfWinamaxPokerLine parseStartOfWinamaxPokerLine(
  std::string_view line) {
  // "^Winamax Poker - .* - HandId: #(.*) .*  - (.*) UTC$"
  if (!line.starts_with("Winamax Poker")) { throw "a Winamax poker line should start with 'Winamax Poker'"; }

  if (!line.ends_with("UTC")) { throw "a Winamax poker line should end with 'UTC'"; }

  const auto datePos { line.rfind(" - ") + MINUS_LENGTH }; // nb char without '\0'

  if (datePos >= line.length()) { throw "bad datePos"; }

  const Time handStartDate({ .strTime = line.substr(datePos, line.rfind(' ') - datePos), .format = WINAMAX_HISTORY_TIME_FORMAT });
  const auto handIdPos { line.find(" - HandId: #") + HAND_ID_LENGTH };
  const auto handId { line.substr(handIdPos, line.find(" - ", handIdPos) - handIdPos) };
  return { .m_handIdPos = handIdPos, .m_handStartDate = handStartDate, .m_handId = std::string(handId) };
}

static constexpr auto BUY_IN_LENGTH { ps::length(" buyIn: ") }; // nb char without '\0
static constexpr auto LEVEL_LENGTH { ps::length(" level: ") }; // nb char without '\0

struct [[nodiscard]] BuyInLevelDateHandId final {
  double m_buyIn;
  int m_level;
  Time m_date;
  std::string m_handId;
};

[[nodiscard]] static inline BuyInLevelDateHandId
getBuyInLevelDateHandIdFromTournamentWinamaxPokerLine(std::string_view line) {
  const auto& [handIdPos, handStartDate, handId] { parseStartOfWinamaxPokerLine(line) };
  // "^Winamax Poker - .* buyIn: (.*) level: (.*) - HandId: #(.*) - .* - (.*) UTC$"
  const auto buyInPos { line.find(" buyIn: ") + BUY_IN_LENGTH };
  const auto levelPos { line.find(" level: ") + LEVEL_LENGTH };
  const auto buyIn { ps::toBuyIn(line.substr(buyInPos, levelPos - LEVEL_LENGTH - buyInPos)) };
  const auto level { ps::toInt(line.substr(levelPos, handIdPos - HAND_ID_LENGTH - levelPos)) };
  return { .m_buyIn = buyIn, .m_level = level, .m_date = handStartDate, .m_handId = handId };
}

struct [[nodiscard]] LevelDateHandId final {
  int m_level;
  Time m_date;
  std::string m_handId;
};

[[nodiscard]] static inline LevelDateHandId
getLevelDateHandIdFromTournamentWinamaxPokerLine(std::string_view line) {
  const auto& [handIdPos, handStartDate, handId] { parseStartOfWinamaxPokerLine(line) };
  // "^Winamax Poker - .* buyIn: (.*) level: (.*) - HandId: #(.*) - .* - (.*) UTC$"
  const auto levelPos { line.find(" level: ") + LEVEL_LENGTH };
  const auto level { ps::toInt(line.substr(levelPos, handIdPos - HAND_ID_LENGTH - levelPos)) };
  return { .m_level = level, .m_date = handStartDate, .m_handId = handId };
}

struct [[nodiscard]] SmallBlindBigBlindDateHandId final {
  double m_smallBlind;
  double m_bigBlind;
  Time m_date;
  std::string m_handId;
};

[[nodiscard]] static inline SmallBlindBigBlindDateHandId
getSmallBlindBigBlindDateHandIdFromCashGameWinamaxPokerLine(std::string_view line) {
  const auto& [_, handStartDate, handId] { parseStartOfWinamaxPokerLine(line) };
  // "^Winamax Poker - .* - HandId: #(.*) - .* \\((.*)/(.*)\\) - (.*) UTC$"
  const auto smallBlindPos { line.find('(') + 1 };
  const auto bigBlindPos { line.find('/') + 1 };
  const auto smallBlind { ps::toAmount(line.substr(smallBlindPos, bigBlindPos - 1 - smallBlindPos)) };
  const auto bigBlind { ps::toAmount(line.substr(bigBlindPos, line.find(')') - 1)) };
  return { .m_smallBlind = smallBlind, .m_bigBlind = bigBlind, .m_date = handStartDate, .m_handId = handId };
}

static constexpr auto DEALT_TO_LENGTH { ps::length("Dealt to ") };

[[nodiscard]] static inline std::array<Card, 5> parseHeroCards(TextFile& tf,
    PlayerCache& cache) {
  LOG.debug<"Parsing hero cards for file {}.">(tf.getFileStem());

  if (tf.startsWith("Dealt to ")) {
    const auto& line { tf.getLine() };
    // "^Dealt to (.*) \\[(.*)\\]$"
    const auto& playerName { line.substr(DEALT_TO_LENGTH, line.find(' ', DEALT_TO_LENGTH) - DEALT_TO_LENGTH) };
    cache.setIsHero(playerName);
    const auto& ret { parseCards(line) };
    tf.next();
    return ret;
  }

  return FIVE_NONE_CARDS;
}

[[nodiscard]] static inline std::array<Card, 5> parseBoardCards(TextFile& tf) {
  LOG.debug<"Parsing board cards for file {}.">(tf.getFileStem());
  std::array ret { FIVE_NONE_CARDS };

  while (!tf.lineIsEmpty()) {
    if (tf.startsWith("Board: ")) {
      // "^Board: \\[([\\w\\s]+)\\]$"
      ret = parseCards(tf.getLine());
    }

    tf.next();
  }

  tf.next();
  return ret;
}

[[nodiscard]] static inline Street parseStreet(TextFile& tf) {
  auto street { Street::none }; // the current line can be *** ANTE/BLINDS ***

  if (tf.startsWith("*** PRE-FLOP ***")) { street = Street::preflop; }
  else if (tf.startsWith("*** FLOP ***")) { street = Street::flop; }
  else if (tf.startsWith("*** TURN ***")) { street = Street::turn; }
  else if (tf.startsWith("*** RIVER ***")) { street = Street::river; }
  else if (tf.startsWith("*** SHOW DOWN ***")) { street = Street::river; }

  tf.next();
  return street;
}

static constexpr auto TABLE_LENGTH { ps::length("Table: '") };
static constexpr auto SEAT_NB_LENGTH { ps::length(" Seat #") };

struct [[nodiscard]] NbMaxSeatsTableNameButtonSeat final {
  Seat m_nbMaxSeats;
  std::string m_tableName;
  Seat m_buttonSeat;
};

// returns nbMaxSeats, tableName, buttonSeat
[[nodiscard]] static inline NbMaxSeatsTableNameButtonSeat
getNbMaxSeatsTableNameButtonSeatFromTableLine(
  TextFile& tf) {
  tf.next();
  const auto& line { tf.getLine() };
  LOG.debug<"Parsing table line {}.">(line);
  // Table: 'Frankfurt 11' 9-max (real money) Seat #2 is the button
  // Table: 'Expresso(111550795)#0' 3-max (real money) Seat #1 is the button
  // ^Table: '(.*)' (.*)-max .* Seat #(.*) is the button$
  const auto pos { line.find("' ", TABLE_LENGTH) };
  const auto& tableName { ps::sanitize(line.substr(TABLE_LENGTH, pos - TABLE_LENGTH)) };
  const auto nbMaxSeats { tableSeat::fromString(line.substr(pos + 2, line.find("-max") - pos - 2)) };
  const auto posSharp { line.find(" Seat #") + SEAT_NB_LENGTH };
  const auto& buttonSeatStr { line.substr(posSharp, line.find(" is the button") - posSharp) };
  const auto buttonSeat { tableSeat::fromString(buttonSeatStr) };

  if (line.starts_with("Seat ")) { throw "a Table line should start with 'Seat '"; }

  tf.next();
  return { .m_nbMaxSeats = nbMaxSeats, .m_tableName = tableName, .m_buttonSeat = buttonSeat };
}

static constexpr auto POSTS_ANTE_LENGTH { ps::length(" posts ante ") };

[[nodiscard]] static inline long parseAnte(TextFile& tf) {
  LOG.debug<"Parsing ante for file {}.">(tf.getFileStem());
  // "^(.*) posts m_ante (.*).*$"
  long ret = 0;

  if (const auto posAnte { tf.find(" posts ante ") };  std::string::npos != posAnte) {
    ret = ps::toInt(tf.getLine().substr(posAnte + POSTS_ANTE_LENGTH));
  }

  while (tf.contains(" posts ")) { tf.next(); }

  return ret;
}

struct [[nodiscard]] ActionParams final {
  std::string_view m_playerName;
  ActionType m_type;
  double m_bet;
};

[[nodiscard]] static inline std::optional<ActionParams>
parseLineForActionParams(std::string_view line) {
  std::optional<ActionParams> ret {};

  if (line.ends_with(" folds")) {
    ret = { .m_playerName = line.substr(0, line.rfind(' ')), .m_type = ActionType::fold, .m_bet = 0.0 };
  } else  if (line.ends_with(" checks")) {
    ret = { .m_playerName = line.substr(0, line.rfind(' ')), .m_type = ActionType::check, .m_bet = 0.0 };
  } else if (ps::contains(line, " calls ")) {
    ret = { .m_playerName = line.substr(0, line.find(" calls ")),
            .m_type = ActionType::call,
            .m_bet = ps::toAmount(line.substr(line.rfind(' ') + 1))
          };
  } else  if (ps::contains(line, " bets ")) {
    ret = { .m_playerName = line.substr(0, line.find(" bets ")),
            .m_type = ActionType::bet,
            .m_bet = ps::toAmount(line.substr(line.rfind(' ') + 1))
          };
  } else if (ps::contains(line, " raises ")) {
    ret = { .m_playerName = line.substr(0, line.find(" raises ")),
            .m_type = ActionType::raise,
            .m_bet = ps::toAmount(line.substr(line.rfind(' ') + 1))
          };
  }

  return ret;
}

static constexpr std::array<std::string_view, 6> ACTION_TOKENS { " folds", " checks", " bets ", " calls ", " raises ", " shows " };

[[nodiscard]] static inline std::vector<std::unique_ptr<Action>> parseActions(TextFile& tf,
    Street street,
std::string_view handId) {
  std::vector<std::unique_ptr<Action>> actions;

  while (tf.containsOneOf(ACTION_TOKENS)) {
    const auto& line { tf.getLine() };

    if (const auto & oActionParams { parseLineForActionParams(line) }; oActionParams.has_value()) {
      const auto [playerName, type, bet] { oActionParams.value() };
      actions.push_back(std::make_unique<Action>(Action::Params {
        .handId = handId,
        .playerName = playerName,
        .street = street,
        .type = type,
        .actionIndex = actions.size(),
        .betAmount = bet }));
    }

    tf.next(); // nothing to do for 'shows' action
  }

  return actions;
}

[[nodiscard]] static inline std::array<std::string, 10> parseWinners(TextFile& tf) {
  std::array<std::string, 10> winners;
  auto pos {  std::string::npos };
  std::size_t i { 0 };

  while (std::string::npos != (pos = tf.find(" collected "))) {
    winners.at(i++) = tf.getLine().substr(0, pos);
    tf.next();
  }

  return winners;
}

[[nodiscard]] static inline std::vector<std::unique_ptr<Action>>
    createActionForWinnersWithoutAction(
      std::span<std::string> winners, std::span<std::unique_ptr<Action>> actions, Street street,
std::string_view handId) {
  std::vector<std::unique_ptr<Action>> ret;
  std::ranges::for_each(winners, [&](std::string_view winner) {
    auto isPlayerName { [&](auto & pAction) { return winner == pAction->getPlayerName(); } };

    if (!winner.empty()
        and (std::end(actions) == std::find_if(std::begin(actions), std::end(actions), isPlayerName))) {
      ret.push_back(std::make_unique<Action>(Action::Params {
        .handId = handId,
        .playerName = winner,
        .street = street,
        .type = ActionType::none,
        .actionIndex = actions.size() + ret.size(),
        .betAmount = 0.0}));
    }
  });
  return ret;
}

[[nodiscard]] static inline
std::pair<std::vector<std::unique_ptr<Action>>, std::array<std::string, 10>> parseActionsAndWinners(
TextFile& tf, std::string_view handId) {
  LOG.debug<"Parsing actions and winners for file {}.">(tf.getFileStem());
  std::vector<std::unique_ptr<Action>> actions;
  Street currentStreet = Street::none;

  while (!tf.contains(" collected ")) {
    currentStreet = parseStreet(tf);
    auto currentActions { parseActions(tf, currentStreet, handId) };
    std::move(currentActions.begin(), currentActions.end(), std::back_inserter(actions));
  }

  auto winners { parseWinners(tf) };
  auto additionalActions { createActionForWinnersWithoutAction(winners, actions, currentStreet, handId) };
  std::move(additionalActions.begin(), additionalActions.end(), std::back_inserter(actions));
  return { std::move(actions), winners };
}

template<GameType gameType>
[[nodiscard]] static inline std::unique_ptr<Hand> getHand(TextFile& tf, PlayerCache& cache,
    int level, const Time& date, std::string_view handId) {
  LOG.debug<"Building hand and maxSeats from history file {}.">(tf.getFileStem());
  const auto& [nbMaxSeats, tableName, buttonSeat] { getNbMaxSeatsTableNameButtonSeatFromTableLine(tf) };
  const auto& seatPlayers { parseSeats(tf, cache) };
  std::ranges::for_each(seatPlayers, [&cache](const auto & p) { if (!p.empty()) { cache.addIfMissing(p); } });
  const auto ante { parseAnte(tf) };
  const auto& heroCards { parseHeroCards(tf, cache) };
  auto [actions, winners] { parseActionsAndWinners(tf, handId) };
  const auto& boardCards { parseBoardCards(tf) };
  LOG.debug<"nb actions={}">(actions.size());
  Hand::Params params { .id = handId, .gameType = gameType, .siteName = ProgramInfos::WINAMAX_SITE_NAME,
                        .tableName = tableName, .buttonSeat = buttonSeat, .maxSeats = nbMaxSeats, .level = level,
                        .ante = ante, .startDate = date, .seatPlayers = seatPlayers, .heroCards = heroCards,
                        .boardCards = boardCards, .actions = std::move(actions), .winners = winners };
  return std::make_unique<Hand>(params);
}

std::unique_ptr<Hand> WinamaxHandBuilder::buildCashgameHand(TextFile& tf, PlayerCache& pc) {
  LOG.debug<"Building Cashgame from history file {}.">(tf.getFileStem());
  const auto& [_, date, handId] { parseStartOfWinamaxPokerLine(tf.getLine()) };
  return getHand<GameType::cashGame>(tf, pc, 0, date, handId); // for cashGame, level is zero
}

std::unique_ptr<Hand> WinamaxHandBuilder::buildTournamentHand(TextFile& tf, PlayerCache& pc) {
  LOG.debug<"Building Tournament from history file {}.">(tf.getFileStem());
  const auto& [level, date, handId] { getLevelDateHandIdFromTournamentWinamaxPokerLine(tf.getLine()) };
  return getHand<GameType::tournament>(tf, pc, level, date, handId);
}

std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>>
    WinamaxHandBuilder::buildCashgameHandAndGameData(TextFile& tf, PlayerCache& pc) {
  LOG.debug<"Building Cashgame and game data from history file {}.">(tf.getFileStem());
  const auto& [smallBlind, bigBlind, date, handId] { getSmallBlindBigBlindDateHandIdFromCashGameWinamaxPokerLine(tf.getLine()) };
  auto pHand { getHand<GameType::cashGame>(tf, pc, 0, date, handId) };
  return { std::move(pHand), std::make_unique<GameData>(GameData::Args{.nbMaxSeats = pHand->getMaxSeats(), .smallBlind = smallBlind, .bigBlind = bigBlind, .buyIn = 0, .startDate = pHand->getStartDate() }) };
}

std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>>
    WinamaxHandBuilder::buildTournamentHandAndGameData(
      TextFile& tf,
PlayerCache& pc) {
  LOG.debug<"Building Tournament and game data from history file {}.">(tf.getFileStem());
  const auto& [buyIn, level, date, handId] { getBuyInLevelDateHandIdFromTournamentWinamaxPokerLine(tf.getLine()) };
  auto pHand { getHand<GameType::tournament>(tf, pc, level, date, handId) };
  return { std::move(pHand), std::make_unique<GameData>(GameData::Args{.nbMaxSeats = pHand->getMaxSeats(), .smallBlind = 0, .bigBlind = 0, .buyIn = buyIn, .startDate = pHand->getStartDate()}) };
}