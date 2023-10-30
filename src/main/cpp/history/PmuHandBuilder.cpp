#include "containers/algorithms.hpp"
#include "entities/Action.hpp"
#include "entities/Card.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp" // Time
#include "entities/Player.hpp"
#include "history/GameData.hpp"
#include "history/PmuHandBuilder.hpp"
#include "history/PokerSiteHandBuilder.hpp" // ProgramInfos
#include "filesystem/TextFile.hpp"
#include "language/assert.hpp" // phudAssert
#include "language/limits.hpp" // toInt
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "threads/PlayerCache.hpp"

#include <cctype> // std::isdigit
#include <optional>
#include <string_view>

static Logger LOG { CURRENT_FILE_NAME };

namespace pa = phud::algorithms;
namespace ps = phud::strings;

static constexpr auto HAND_HISTORY_FOR_GAME_SIZE { ps::length("***** Hand History for Game ") };

static inline void seekToHandStart(TextFile& tf) {
  while (!tf.startsWith("***** Hand History for Game ")) { tf.next(); }
}

static inline void seekToDealingDownCards(TextFile& tf) {
  while (!tf.startsWith("** Dealing down cards **")) { tf.next(); }
}

[[nodiscard]] static inline std::string readHandId(std::string_view line) {
  return std::string { line.substr(HAND_HISTORY_FOR_GAME_SIZE, line.rfind(" *****") - HAND_HISTORY_FOR_GAME_SIZE) };
}

[[nodiscard]] bool isDigit(const auto& someGenericChar) { return 0 != std::isdigit(someGenericChar); }

struct InfosFromCashGamePmuPokerLine {
  double m_smallBlind;
  double m_bigBlind;
  Time m_startDate;
  Limit m_limit;
  Variant m_variant;
};

// smallBlind, bigBlind, startDate, limit, variant
[[nodiscard]] static inline InfosFromCashGamePmuPokerLine
getInfosFromCashGamePmuPokerLine(std::string_view line) {
  // €0.01/€0.02 EUR NL Texas Hold'em - Tuesday, September 21, 10:45:33 CEST 2021
  const auto smallBlindPos { line.find_first_of(".0123456789", 1) };
  const auto bigBlindPos { line.find_first_of(".0123456789", line.find('/') + 1) };
  const auto smallBlind { ps::toAmount(line.substr(smallBlindPos, bigBlindPos - smallBlindPos - 1)) };
  const auto bigBlind { ps::toAmount(line.substr(bigBlindPos, line.find(' '))) };
  const auto limit { ps::contains(line, " NL ") ? Limit::noLimit : Limit::potLimit };
  const auto variant { ps::contains(line, " Texas Hold'em ") ? Variant::holdem : Variant::omaha };
  std::string dateStr { line.substr(line.find(" - ") + ps::length(" - ")) };
  // remove the timezone as it is not parsable as of today
  const auto lastSpacePos { dateStr.rfind(' ') };
  const auto secondLastSpacePos { dateStr.rfind(' ', lastSpacePos - 1) };
  dateStr.replace(secondLastSpacePos, lastSpacePos - secondLastSpacePos, "");
  const Time date({ .strTime = dateStr, .format = PMU_HISTORY_TIME_FORMAT });
  return { .m_smallBlind = smallBlind, .m_bigBlind = bigBlind, .m_startDate = date, .m_limit = limit, .m_variant = variant };
}

static constexpr auto TABLE_LENGTH { ps::length("Table ") };

// tableName, isRealMoney
[[nodiscard]] static inline std::pair<std::string, bool> parseTableLine(std::string_view line) {
  const std::string tableName { line.substr(TABLE_LENGTH, line.find(" (") - TABLE_LENGTH) };
  return { tableName, line.ends_with("(Real Money)") };
}

[[nodiscard]] static inline Seat parseButtonSeatLine(std::string_view line) {
  return tableSeat::fromString(line.substr(SEAT_LENGTH,
                               line.find(" is the button") - SEAT_LENGTH));
}

[[nodiscard]] static inline Seat parseTotalNumberOfPlayersLine(std::string_view line) {
  return tableSeat::fromString(line.substr(line.find('/') + 1));
}

[[nodiscard]] static inline double parseAnte(TextFile& /*tf*/) {
  return 0; // TODO
}

[[nodiscard]] static std::array<Card, 5> parseCards(std::string_view line, std::string_view cardDelimiter) {
  // [  Jd 3c ] (yep, 2 spaces) or [ 4h, 3h, Jh ] or [ Kh ]
  const auto pos { line.find_first_not_of(" ", line.rfind('[') + 1) };
  const auto strCards { line.substr(pos, line.rfind(" ]") - pos) };
  const auto& cardsStr { split(strCards, cardDelimiter) };
  std::array<Card, 5> ret;
  std::transform(cardsStr.begin(), cardsStr.end(), ret.begin(), toCard);
  return ret;
}

static constexpr std::array<Card, 5> FIVE_NONE_CARDS { Card::none, Card::none, Card::none, Card::none, Card::none };
static constexpr auto DEALT_TO_LENGTH { ps::length("Dealt to ") };

[[nodiscard]] static constexpr std::array<Card, 5> parseHeroCards(std::string_view line,
    PlayerCache& cache) {
  // it is possible that hero is present at the table but do not play
  if (line.starts_with("Dealt to ")) {
    // "^Dealt to (.*) \\[(.*)\\]$"
    const auto playerName { line.substr(DEALT_TO_LENGTH, line.find(' ', DEALT_TO_LENGTH) - DEALT_TO_LENGTH) };
    cache.setIsHero(playerName);
    return parseCards(line, " ");
  }

  return FIVE_NONE_CARDS;
}

[[nodiscard]] static inline std::pair<Street, std::array<Card, 5>> parseStreet(std::string_view line) {
  if (line.starts_with("** Dealing Flop **")) {
    return { Street::flop, parseCards(line, ", ") };
  } else if (line.starts_with("** Dealing Turn **")) {
    auto cards { parseCards(line, " ") };
    std::rotate(cards.rbegin(), cards.rbegin() + 3, cards.rend());
    return { Street::turn, cards };
  } else if (line.starts_with("** Dealing River **")) {
    auto cards { parseCards(line, " ") };
    std::rotate(cards.rbegin(), cards.rbegin() + 4, cards.rend());
    return { Street::river, cards };
  }

  return { Street::preflop, FIVE_NONE_CARDS };
}

struct LineForActionParams {
  std::string_view m_playerName;
  ActionType m_type;
  double m_bet;
};

[[nodiscard]] static inline std::optional<LineForActionParams>
parseLineForActionParams(std::string_view line) {
  std::optional<LineForActionParams> ret {};

  // TODO factoriser
  if (line.ends_with(" folds")) {
    ret = { .m_playerName = line.substr(0, line.rfind(' ')), .m_type = ActionType::fold, .m_bet = 0.0 };
  } else if (line.ends_with(" checks")) {
    ret = { .m_playerName = line.substr(0, line.rfind(' ')), .m_type = ActionType::check, .m_bet = 0.0 };
  } else if (ps::contains(line, " calls ")) {
    ret = { .m_playerName = line.substr(0, line.find(" calls ")), .m_type = ActionType::call, .m_bet = ps::toAmount(line.substr(line.find_first_of(".0123456789"), line.rfind(' '))) };
  } else if (ps::contains(line, " bets ")) {
    ret = { .m_playerName = line.substr(0, line.find(" bets ")), .m_type = ActionType::bet, .m_bet = ps::toAmount(line.substr(line.find_first_of(".0123456789"), line.rfind(' '))) };
  } else if (ps::contains(line, " raises ")) {
    ret = { .m_playerName = line.substr(0, line.find(" raises ")), .m_type = ActionType::raise,
            .m_bet = ps::toAmount(line.substr(line.find_first_of(".0123456789"), line.rfind(' ')))
          };
  } else  if (ps::contains(line, " is all-In ")) {
    ret = { .m_playerName = line.substr(0, line.find(" is all-In ")), .m_type = ActionType::raise,
            .m_bet = ps::toAmount(line.substr(line.find_first_of(".0123456789"), line.rfind(' ')))
          };
  }

  return ret;
}

static constexpr std::array<std::string_view, 8> ACTION_TOKENS { " folds", " checks", " bets ", " calls ", " raises ", " is all-In ", " shows ", " doesn't show " };

[[nodiscard]] static inline std::vector<std::unique_ptr<Action>>
parseActions(TextFile& tf, Street street, std::string_view handId) {
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
  auto pos { std::string::npos };
  std::size_t i { 0 };

  while (std::string::npos != (pos = tf.find(" wins "))) {
    winners.at(i++) = tf.getLine().substr(0, pos);
    tf.next();
  }

  return winners;
}

[[nodiscard]] static inline std::vector<std::unique_ptr<Action>> createActionForWinnersWithoutAction(
std::span<const std::string> winners, std::span<std::unique_ptr<Action>> actions, Street street, std::string_view handId) {
  std::vector<std::unique_ptr<Action>> ret;
  pa::forEach(winners, [&](std::string_view winner) {
    if (!winner.empty() and !pa::containsIf(actions, [&](auto & pAction) { return winner == pAction->getPlayerName(); })) {
      ret.push_back(std::make_unique<Action>(Action::Params {
        .handId = handId,
        .playerName = winner,
        .street = street,
        .type = ActionType::none,
        .actionIndex = actions.size() + ret.size(),
        .betAmount = 0.0 }));
    }
  });
  return ret;
}

std::array<Card, 5> getBoardCards(Street street, const std::array<Card, 5>& cards,
                                  const std::array<Card, 5>& previous) {
  switch (street) {
    case Street::none: [[fallthrough]];

    case Street::preflop: { return { previous.at(0), previous.at(1), previous.at(2), previous.at(3), previous.at(4) }; }

    case Street::flop: { return { cards.at(0), cards.at(1), cards.at(2), previous.at(3), previous.at(4) }; }

    case Street::turn: { return { previous.at(0), previous.at(1), previous.at(2), cards.at(3), previous.at(4) }; }

    case Street::river: { return { previous.at(0), previous.at(1), previous.at(2), previous.at(3), cards.at(4) }; }

    default: { phudAssert(false, "Unknown street !!!"); }
  }

  return FIVE_NONE_CARDS;
}

struct ActionsAndWinnersAndBoardCards {
  std::vector<std::unique_ptr<Action>> m_actions;
  std::array<std::string, 10> m_winners;
  std::array<Card, 5> m_boardCards;
};

// actions, winners,board cards
[[nodiscard]] static inline ActionsAndWinnersAndBoardCards
parseActionsAndWinnersAndBoardCards(TextFile& tf, std::string_view handId) {
  LOG.debug<"Parsing actions and winners and boardcards for file {}.">(tf.getFileStem());
  std::vector<std::unique_ptr<Action>> actions;
  std::array boardCards { FIVE_NONE_CARDS };
  Street lastStreet { Street::none };

  while (!tf.contains(" wins ")) {
    const auto& [currentStreet, streetCards] { parseStreet(tf.getLine()) };

    if (Street::preflop != currentStreet) {
      boardCards = getBoardCards(currentStreet, streetCards, boardCards);
    }

    tf.next();
    auto currentActions { parseActions(tf, currentStreet, handId) };
    pa::moveInto(currentActions, actions);
    lastStreet = currentStreet;
  }

  const auto& winners { parseWinners(tf) };
  auto additionalActions { createActionForWinnersWithoutAction(winners, actions, lastStreet, handId) };
  pa::moveInto(additionalActions, actions);
  return { .m_actions = std::move(actions), .m_winners = winners, .m_boardCards = boardCards };
}

static constexpr auto SEAT_NB_LENGTH { ps::length(" Seat #") };

struct NbMaxSeatsTableNameButtonSeat {
  int m_nbMaxSeats;
  std::string m_tableName;
  int m_buttonSeat;
};

// nbMaxSeats, tableName, buttonSeat
NbMaxSeatsTableNameButtonSeat getNbMaxSeatsTableNameButtonSeatFromTableLine(TextFile& tf) {
  tf.next();
  const auto& line { tf.getLine() };
  LOG.debug<"Parsing table line {}.">(line);
  // Table: 'Frankfurt 11' 9-max (real money) Seat #2 is the button
  // Table: 'Expresso(111550795)#0' 3-max (real money) Seat #1 is the button
  // ^Table: '(.*)' (.*)-max .* Seat #(.*) is the button$
  const auto pos { line.find("' ", TABLE_LENGTH) };
  const auto& tableName { ps::sanitize(line.substr(TABLE_LENGTH, pos - TABLE_LENGTH)) };
  const auto nbMaxSeats { ps::toInt(line.substr(pos + 2, line.find("-max") - pos - 2)) };
  const auto posSharp { line.find(" Seat #") + SEAT_NB_LENGTH };
  const auto& buttonSeatStr { line.substr(posSharp, line.find(" is the button") - posSharp) };
  const auto buttonSeat { ps::toInt(buttonSeatStr) };

  if (line.starts_with("Seat ")) { throw "a Table line should start with 'Seat '"; }

  tf.next();
  return { .m_nbMaxSeats = nbMaxSeats, .m_tableName = tableName, .m_buttonSeat = buttonSeat };
}

template<GameType gameType>
[[nodiscard]] static inline std::unique_ptr<Hand> getHand(TextFile& tf, PlayerCache& cache,
    int level, const Time& date, std::string_view handId) {
  LOG.debug<"Building hand and maxSeats from history file {}.">(tf.getFileStem());
  const auto& [nbMaxSeats, tableName, buttonSeat] { getNbMaxSeatsTableNameButtonSeatFromTableLine(tf) };
  const auto& seatPlayers { parseSeats(tf, cache) };
  const auto ante { parseAnte(tf) };
  const auto& heroCards { parseHeroCards(tf.getLine(), cache) };
  auto [actions, winners, boardCards] { std::move(parseActionsAndWinnersAndBoardCards(tf, handId)) };
  LOG.debug<"nb actions={}">(actions.size());
  Hand::Params params { .id = handId, .gt = gameType, .siteName = ProgramInfos::PMU_SITE_NAME,
                        .tableName = tableName, .buttonSeat = buttonSeat, .maxSeats = nbMaxSeats, .level = level,
                        .ante = ante, .startDate = date, .seatPlayers = seatPlayers, .heroCards = heroCards,
                        .boardCards = boardCards, .actions = std::move(actions), .winners = winners };
  return std::make_unique<Hand>(params);
}

std::unique_ptr<Hand> PmuHandBuilder::buildCashgameHand(TextFile& tf, PlayerCache& cache) {
  LOG.debug<"Building Cashgame and game data from history file {}.">(tf.getFileStem());
  seekToHandStart(tf);
  const auto handId { readHandId(tf.getLine()) };
  tf.next();
  const auto& [_1, _2, startDate, _3, _4] { getInfosFromCashGamePmuPokerLine(tf.getLine()) };// TODO simplify
  tf.next();
  const auto& [tableName, _5] { parseTableLine(tf.getLine()) }; // TODO unneeded
  tf.next();
  const auto buttonSeat { parseButtonSeatLine(tf.getLine()) };
  tf.next();
  const auto nbMaxSeats { parseTotalNumberOfPlayersLine(tf.getLine()) }; // TODO unneeded
  tf.next();
  const auto& seatPlayers { parseSeats(tf, cache) };
  pa::forEach(seatPlayers, [&cache](const auto & p) { if (!p.empty()) { cache.addIfMissing(p); } });
  seekToDealingDownCards(tf);
  tf.next();
  const auto& heroCards { parseHeroCards(tf.getLine(), cache) };
  auto [actions, winners, boardCards] { std::move(parseActionsAndWinnersAndBoardCards(tf, handId)) };
  LOG.debug<"nb actions={}">(actions.size());
  Hand::Params p { .id = handId, .gameType = GameType::cashGame, .siteName = ProgramInfos::PMU_SITE_NAME, .tableName = tableName,
                   .buttonSeat = buttonSeat, .maxSeats = nbMaxSeats, .level = 0, .ante = 0, .startDate = startDate, .seatPlayers = seatPlayers,
                   .heroCards = heroCards, .boardCards = boardCards, .actions = std::move(actions), .winners = winners };
  return std::make_unique<Hand>(p);
}

std::unique_ptr<Hand> PmuHandBuilder::buildTournamentHand(TextFile& /*tf*/, PlayerCache& /*cache*/) {
  // not implemented
  return nullptr;
}

std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>> PmuHandBuilder::buildCashgameHandAndGameData(TextFile& tf,
PlayerCache& cache) {
  LOG.debug<"Building Cashgame and game data from history file {}.">(tf.getFileStem());
  seekToHandStart(tf);
  tf.next();  // during the 1rst hand, we don't know the names of our opponents
  seekToHandStart(tf);
  const auto handId { readHandId(tf.getLine()) };
  tf.next();
  const auto& [smallBlind, bigBlind, startDate, limit, variant] { getInfosFromCashGamePmuPokerLine(tf.getLine()) };
  tf.next();
  const auto& [tableName, isRealMoney] { parseTableLine(tf.getLine()) };
  tf.next();
  const auto buttonSeat { parseButtonSeatLine(tf.getLine()) };
  tf.next();
  const auto nbMaxSeats { parseTotalNumberOfPlayersLine(tf.getLine()) };
  tf.next();
  const auto& seatPlayers { parseSeats(tf, cache) };
  seekToDealingDownCards(tf);
  tf.next();
  const auto& heroCards { parseHeroCards(tf.getLine(), cache) };
  auto [actions, winners, boardCards] { std::move(parseActionsAndWinnersAndBoardCards(tf, handId)) };
  LOG.debug<"nb actions={}">(actions.size());
  Hand::Params params { .id = handId, .gameType = GameType::cashGame, .siteName = ProgramInfos::PMU_SITE_NAME, .tableName = tableName,
                        .buttonSeat = buttonSeat, .maxSeats = nbMaxSeats, .level = 0, .ante = 0, .startDate = startDate, .seatPlayers = seatPlayers,
                        .heroCards = heroCards, .boardCards = boardCards, .actions = std::move(actions), .winners = winners };
  auto pHand { std::make_unique<Hand>(params) };
  auto pGameData { std::make_unique<GameData>(GameData::Args{.nbMaxSeats = pHand->getMaxSeats(), .smallBlind = smallBlind, .bigBlind = bigBlind, .buyIn = 0, .startDate = pHand->getStartDate()}) };
  pGameData->m_isRealMoney = isRealMoney;
  pGameData->m_tableName = tableName;
  pGameData->m_gameName = tableName;
  pGameData->m_variant = variant;
  pGameData->m_limit = limit;
  return { std::move(pHand), std::move(pGameData) };
}

std::pair<std::unique_ptr<Hand>, std::unique_ptr<GameData>> PmuHandBuilder::buildTournamentHandAndGameData(TextFile& /*tf*/,
PlayerCache& /*cache*/) {
  //LOG.debug<"Building Tournament and game data from history file {}.">(tf.getFileStem());
  //const auto& [buyIn, level, date, handId] { getBuyInLevelDateHandIdFromTournamentWinamaxPokerLine(tf.getLine()) };
  //auto pHand { getHand<GameType::tournament>(tf, pc, level, date, handId) };
  //return { std::move(pHand), std::make_unique<GameData>(pHand->getStartDate(), buyIn, 0, 0, pHand->getMaxSeats()) };
  return { nullptr, nullptr };
}