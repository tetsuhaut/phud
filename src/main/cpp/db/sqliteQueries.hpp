#pragma once

#include <string_view>

namespace phud::sql {
static constexpr std::string_view CREATE_SITE { R"raw(
CREATE TABLE Site (
  siteName TEXT NOT NULL PRIMARY KEY
);
)raw" };

static constexpr std::string_view CREATE_HAND { R"raw(
CREATE TABLE Hand (
  handId TEXT NOT NULL PRIMARY KEY, 
  siteName TEXT NOT NULL, 
  tableName TEXT NOT NULL, 
  buttonSeat INT NOT NULL, 
  maxSeats INT NOT NULL, 
  level INT NOT NULL, 
  ante INT NOT NULL, 
  startDate DATETIME NOT NULL, 
  heroCard1 TEXT NOT NULL, 
  heroCard2 TEXT NOT NULL, 
  heroCard3 TEXT, 
  heroCard4 TEXT, 
  heroCard5 TEXT, 
  boardCard1 TEXT, 
  boardCard2 TEXT, 
  boardCard3 TEXT, 
  boardCard4 TEXT, 
  boardCard5 TEXT, 
  FOREIGN KEY(siteName) REFERENCES Site(siteName)
);
)raw" };

static constexpr std::string_view CREATE_GAME { R"raw(
CREATE TABLE Game (
  gameId TEXT NOT NULL PRIMARY KEY, 
  siteName TEXT  NOT NULL, 
  gameName TEXT NOT NULL, 
  variant TEXT  NOT NULL, 
  limitType TEXT NOT NULL, 
  isRealMoney BOOLEAN  NOT NULL, 
  nbMaxSeats INT NOT NULL, 
  startDate DATETIME NOT NULL, 
  FOREIGN KEY(siteName) REFERENCES Site(siteName)
);
)raw" };

static constexpr std::string_view CREATE_CASH_GAME { R"raw(
CREATE TABLE CashGame (
  cashGameId TEXT NOT NULL PRIMARY KEY, 
  smallBlind REAL NOT NULL, 
  bigBlind REAL NOT NULL, 
  FOREIGN KEY(cashGameId) REFERENCES Game(gameId)
);
)raw" };

static constexpr std::string_view CREATE_TOURNAMENT { R"raw(
CREATE TABLE Tournament (
  tournamentId TEXT NOT NULL PRIMARY KEY, 
  prizepool INT, 
  buyIn REAL, 
  startingStack INT, 
  rebuy BOOLEAN, 
  rebuyCost INT, 
  rebuyChips INT, 
  addOn BOOLEAN, 
  addOnCost INT, 
  addOnChips INT, 
  knockout BOOLEAN, 
  speed TEXT, 
  shootout BOOLEAN, 
  reEntry BOOLEAN, 
  seatNGo BOOLEAN, 
  canTime BOOLEAN, 
  timeAmount INT, 
  satellite BOOLEAN, 
  doubleOrNothing BOOLEAN, 
  FOREIGN KEY(tournamentId) REFERENCES Game(gameId)
);
)raw" };

static constexpr std::string_view CREATE_CASH_GAME_HAND { R"raw(
CREATE TABLE CashGameHand (
  cashGameHandId INTEGER PRIMARY KEY, 
  cashGameId TEXT NOT NULL, 
  handId INT NOT NULL, 
  FOREIGN KEY(cashGameId) REFERENCES CashGame(cashGameId), 
  FOREIGN KEY(cashGameHandId) REFERENCES Hand(handId)
);
)raw" };

static constexpr std::string_view CREATE_TOURNAMENT_HAND { R"raw(
CREATE TABLE TournamentHand (
  tournamentHandId INTEGER PRIMARY KEY, 
  tournamentId TEXT NOT NULL, 
  handId INT NOT NULL, 
  FOREIGN KEY(tournamentId) REFERENCES Tournament(tournamentId), 
  FOREIGN KEY(tournamentHandId) REFERENCES Hand(handId)
);
)raw" };

static constexpr std::string_view CREATE_ACTION { R"raw(
CREATE TABLE Action (
  actionId INTEGER PRIMARY KEY, 
  street TEXT NOT NULL, 
  handId TEXT NOT NULL, 
  playerName TEXT NOT NULL, 
  actionType TEXT NOT NULL, 
  actionIndex INT NOT NULL, 
  betAmount REAL NOT NULL, 
  FOREIGN KEY(handId) REFERENCES Hand(handId)
);
)raw" };

static constexpr std::string_view CREATE_PLAYER { R"raw(
CREATE TABLE Player (
  playerName TEXT NOT NULL, 
  siteName TEXT NOT NULL, 
  isHero BOOLEAN, 
  comments TEXT, 
  PRIMARY KEY(playerName, siteName), 
  FOREIGN KEY(siteName) REFERENCES Site(siteName)
);
)raw" };

static constexpr std::string_view CREATE_HAND_PLAYER { R"raw(
CREATE TABLE HandPlayer (
  handId TEXT NOT NULL, 
  playerName TEXT NOT NULL, 
  playerSeat INT NOT NULL, 
  isWinner BOOLEAN, 
  PRIMARY KEY(handId, playerName), 
  FOREIGN KEY(handId) REFERENCES Hand(handId), 
  FOREIGN KEY(playerName) REFERENCES Player(playerName)
);
)raw" };

static constexpr std::string_view INSERT_SITE { R"raw(
INSERT OR IGNORE INTO Site (siteName) VALUES ('?siteName');
)raw" };

static constexpr std::string_view INSERT_GAME { R"raw(
INSERT OR IGNORE INTO Game (
  gameId, siteName, gameName, variant, limitType, isRealMoney, nbMaxSeats, startDate
)
VALUES (
  '?gameId', '?siteName', '?gameName', '?variant', '?limitType', ?isRealMoney, ?nbMaxSeats, '?startDate'
);
)raw" };

static constexpr std::string_view INSERT_TOURNAMENT {
  "INSERT OR IGNORE INTO Tournament (tournamentId, buyIn) VALUES ('?tournamentId', ?buyIn);"
};

static constexpr std::string_view INSERT_CASHGAME { R"raw(
INSERT OR IGNORE INTO CashGame (cashGameId, smallBlind, bigBlind)
VALUES ('?cashGameId', ?smallBlind, ?bigBlind);
)raw" };

static constexpr std::string_view INSERT_TOURNAMENT_HAND {
  "INSERT OR IGNORE INTO TournamentHand (tournamentId, handId) VALUES ('?gameId', '?handId');"
};

static constexpr std::string_view INSERT_CASHGAME_HAND {
  "INSERT OR IGNORE INTO CashGameHand (cashGameId, handId) VALUES ('?gameId', '?handId');"
};

static constexpr std::string_view INSERT_HAND { R"raw(
INSERT OR IGNORE INTO Hand (
  handId, siteName, tableName, buttonSeat, maxSeats, ante, level, startDate,
  heroCard1, heroCard2, heroCard3, heroCard4, heroCard5,
  boardCard1, boardCard2, boardCard3, boardCard4, boardCard5
)
VALUES (
  '?handId', '?siteName', '?tableName', ?buttonSeat, ?maxSeats, ?ante, ?level,
  '?startDate', '?heroCard1', '?heroCard2', '?heroCard3', '?heroCard4', '?heroCard5',
  '?boardCard1', '?boardCard2', '?boardCard3', '?boardCard4', '?boardCard5'
);
)raw" };

/**
  * Inserts an Action entity into the Action table.
  * @param street as std::string
  * @param handId as std::string
  * @param playerName as std::string
  * @param actionType as std::string
  * @param actionIndex as int
  * @param betAmount as double
  */
static constexpr std::string_view INSERT_ACTION { R"raw(
INSERT OR IGNORE INTO Action (
  street, handId, playerName, actionType, actionIndex, betAmount
)
VALUES (
  '?street', '?handId', '?playerName', '?actionType', ?actionIndex, ?betAmount
);
)raw" };

static constexpr std::string_view INSERT_PLAYER { R"raw(
INSERT OR IGNORE INTO Player (playerName, siteName, isHero, comments)
VALUES ('?playerName', '?siteName', ?isHero, '?comments');
)raw" };

static constexpr std::string_view INSERT_HAND_PLAYER { R"raw(
INSERT OR IGNORE INTO HandPlayer (handId, playerName, playerSeat, isWinner)
VALUES ('?handId', '?playerName', ?playerSeat, ?isWinner);
)raw" };

/**
 * Retrieves the different statistics for a given player, currently
 * - Voluntary Put Money In Pot
 * - Pre Flop Raise
 * @param siteName
 * @param playerName
 * @return columns isHero, comments, nbHands, VPIP, PFR
 */
static constexpr std::string_view GET_STATS_BY_SITE_AND_PLAYER_NAME { R"raw(
SELECT
  p.isHero,
  100 * COUNT(CASE WHEN(a.actionType in('call','bet','raise')) then 1 END) / (1.0 * COUNT(1)) as VPIP,
  100 * COUNT(CASE WHEN(a.actionType = 'raise') then 1 END) / (1.0 * COUNT(1)) as PFR,
  COUNT(1) as nbHands 
FROM Action a 
JOIN Player p ON p.playerName = a.playerName
WHERE
  p.playerName = '?playerName' AND
  p.siteName = '?siteName' AND
  a.street = 'preflop';
)raw" };


/**
 * Retrieves the different statistics for the given table, currently
 * - Voluntary Put Money In Pot
 * - Pre Flop Raise
 * @param siteName
 * @param tableName
 * @return columns isHero, comments, nbHands, VPIP, PFR
 */
static constexpr std::string_view GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME { R"raw(
SELECT
  p.playerName, p.siteName, p.isHero, hp.playerSeat, p.comments,
  100 * COUNT(CASE WHEN(a.actionType IN ('call','bet','raise')) then 1 END) / (1.0 * COUNT(1)) as VPIP,
  100 * COUNT(CASE WHEN(a.actionType = 'raise') then 1 END) / (1.0 * COUNT(1)) as PFR,
  COUNT(1) as nbHands
FROM (
  SELECT * FROM Hand h WHERE
  h.tableName = '?tableName' AND h.siteName = '?siteName'
  ORDER BY h.startDate DESC limit 1
) h
JOIN Player p ON h.siteName = p.siteName
JOIN HandPlayer hp ON hp.playerName = p.playerName AND hp.handId = h.handId
LEFT JOIN Action a ON p.playerName = a.playerName AND a.street = 'preflop'
WHERE
h.siteName = '?siteName'
GROUP BY p.playerName ORDER BY playerSeat;
)raw" };

static constexpr std::string_view GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME { R"raw(
SELECT
  maxSeats
FROM
  Hand h
WHERE
  h.tableName = '?tableName' AND h.siteName = '?siteName'
ORDER BY h.startDate DESC limit 1
)raw" };

static constexpr std::string_view CREATE_QUERIES[] {
  CREATE_SITE, CREATE_HAND, CREATE_GAME, CREATE_CASH_GAME, CREATE_TOURNAMENT,
  CREATE_CASH_GAME_HAND, CREATE_TOURNAMENT_HAND, CREATE_ACTION, CREATE_PLAYER,
  CREATE_HAND_PLAYER
};

} // namespace phud::sql
