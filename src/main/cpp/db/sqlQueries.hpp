#pragma once

#include <array>
#include <string_view>

namespace phud::sql {
  static constexpr std::string_view CREATE_SITE = R"raw(
CREATE TABLE Site (
  siteName TEXT NOT NULL PRIMARY KEY
);
)raw";

  static constexpr std::string_view CREATE_HAND = R"raw(
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
)raw";

  static constexpr std::string_view CREATE_GAME = R"raw(
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
)raw";

  static constexpr std::string_view CREATE_CASH_GAME = R"raw(
CREATE TABLE CashGame (
  cashGameId TEXT NOT NULL PRIMARY KEY, 
  smallBlind REAL NOT NULL, 
  bigBlind REAL NOT NULL, 
  FOREIGN KEY(cashGameId) REFERENCES Game(gameId)
);
)raw";

  static constexpr std::string_view CREATE_TOURNAMENT = R"raw(
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
)raw";

  static constexpr std::string_view CREATE_CASH_GAME_HAND = R"raw(
CREATE TABLE CashGameHand (
  cashGameHandId INTEGER PRIMARY KEY, 
  cashGameId TEXT NOT NULL, 
  handId INT NOT NULL, 
  FOREIGN KEY(cashGameId) REFERENCES CashGame(cashGameId), 
  FOREIGN KEY(cashGameHandId) REFERENCES Hand(handId)
);
)raw";

  static constexpr std::string_view CREATE_TOURNAMENT_HAND = R"raw(
CREATE TABLE TournamentHand (
  tournamentHandId INTEGER PRIMARY KEY, 
  tournamentId TEXT NOT NULL, 
  handId INT NOT NULL, 
  FOREIGN KEY(tournamentId) REFERENCES Tournament(tournamentId), 
  FOREIGN KEY(tournamentHandId) REFERENCES Hand(handId)
);
)raw";

  static constexpr std::string_view CREATE_ACTION = R"raw(
CREATE TABLE Action (
  actionId INTEGER PRIMARY KEY, 
  street TEXT NOT NULL, 
  handId TEXT NOT NULL, 
  playerName TEXT NOT NULL, 
  actionType TEXT NOT NULL, 
  actionIndex INT NOT NULL,
  amount REAL NOT NULL,
  UNIQUE(handId, actionIndex),
  FOREIGN KEY(handId) REFERENCES Hand(handId)
);
)raw";

  static constexpr std::string_view CREATE_PLAYER = R"raw(
CREATE TABLE Player (
  playerName TEXT NOT NULL, 
  siteName TEXT NOT NULL, 
  isHero BOOLEAN, 
  comments TEXT, 
  PRIMARY KEY(playerName, siteName), 
  FOREIGN KEY(siteName) REFERENCES Site(siteName)
);
)raw";

  static constexpr std::string_view CREATE_HAND_PLAYER = R"raw(
CREATE TABLE HandPlayer (
  handId TEXT NOT NULL, 
  playerName TEXT NOT NULL, 
  playerSeat INT NOT NULL, 
  isWinner BOOLEAN, 
  PRIMARY KEY(handId, playerName), 
  FOREIGN KEY(handId) REFERENCES Hand(handId), 
  FOREIGN KEY(playerName) REFERENCES Player(playerName)
);
)raw";

  /**
   * What each player did in each hand, filled when saving the hands and thus never updated. The
   * primary key makes saving the same hand again harmless.
   */
  static constexpr std::string_view CREATE_HAND_PLAYER_STATS = R"raw(
CREATE TABLE HandPlayerStats (
  handId TEXT NOT NULL,
  playerName TEXT NOT NULL,
  siteName TEXT NOT NULL,
  gameType TEXT NOT NULL,
  vpip BOOLEAN NOT NULL,
  pfr BOOLEAN NOT NULL,
  limp BOOLEAN NOT NULL,
  facedRaise BOOLEAN NOT NULL,
  foldToRaise BOOLEAN NOT NULL,
  callRaise BOOLEAN NOT NULL,
  threeBet BOOLEAN NOT NULL,
  couldSqueeze BOOLEAN NOT NULL,
  squeeze BOOLEAN NOT NULL,
  facedThreeBet BOOLEAN NOT NULL,
  foldToThreeBet BOOLEAN NOT NULL,
  callThreeBet BOOLEAN NOT NULL,
  fourBet BOOLEAN NOT NULL,
  PRIMARY KEY(handId, playerName),
  FOREIGN KEY(handId) REFERENCES Hand(handId),
  FOREIGN KEY(playerName) REFERENCES Player(playerName)
);
)raw";

  static constexpr std::string_view CREATE_HAND_PLAYER_STATS_INDEX = R"raw(
CREATE INDEX HandPlayerStatsByPlayer ON HandPlayerStats (playerName, siteName, gameType);
)raw";

  static constexpr std::string_view INSERT_SITE = R"raw(
INSERT OR IGNORE INTO Site (siteName) VALUES ('?siteName');
)raw";

  static constexpr std::string_view INSERT_GAME = R"raw(
INSERT OR IGNORE INTO Game (
  gameId, siteName, gameName, variant, limitType, isRealMoney, nbMaxSeats, startDate
)
VALUES (
  '?gameId', '?siteName', '?gameName', '?variant', '?limitType', ?isRealMoney, ?nbMaxSeats, '?startDate'
);
)raw";

  static constexpr std::string_view INSERT_TOURNAMENT = R"raw(
    INSERT OR IGNORE INTO Tournament (tournamentId, buyIn) VALUES ('?tournamentId', ?buyIn);
)raw";

  static constexpr std::string_view INSERT_CASHGAME = R"raw(
INSERT OR IGNORE INTO CashGame (cashGameId, smallBlind, bigBlind)
VALUES ('?cashGameId', ?smallBlind, ?bigBlind);
)raw";

  static constexpr std::string_view INSERT_TOURNAMENT_HAND = R"raw(
    INSERT OR IGNORE INTO TournamentHand (tournamentId, handId) VALUES ('?gameId', '?handId');
)raw";

  static constexpr std::string_view INSERT_CASHGAME_HAND = R"raw(
    INSERT OR IGNORE INTO CashGameHand (cashGameId, handId) VALUES ('?gameId', '?handId');
)raw";

  static constexpr std::string_view INSERT_HAND = R"raw(
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
)raw";

  /**
   * This parameterized query inserts an Action entity into the Action table.
   * param street as std::string
   * param handId as std::string
   * param playerName as std::string
   * param actionType as std::string
   * param actionIndex as int
   * param amount as double
   */
  static constexpr std::string_view INSERT_ACTION = R"raw(
INSERT OR IGNORE INTO Action (
  street, handId, playerName, actionType, actionIndex, amount
)
VALUES (
  '?street', '?handId', '?playerName', '?actionType', ?actionIndex, ?amount
);
)raw";

  static constexpr std::string_view INSERT_PLAYER = R"raw(
INSERT OR IGNORE INTO Player (playerName, siteName, isHero, comments)
VALUES ('?playerName', '?siteName', ?isHero, '?comments');
)raw";

  static constexpr std::string_view INSERT_HAND_PLAYER = R"raw(
INSERT OR IGNORE INTO HandPlayer (handId, playerName, playerSeat, isWinner)
VALUES ('?handId', '?playerName', ?playerSeat, ?isWinner);
)raw";

  /**
   * Write the player statistics that are shown by the HUD
   */
  static constexpr std::string_view INSERT_HAND_PLAYER_STATS = R"raw(
INSERT OR IGNORE INTO HandPlayerStats (
  handId, playerName, siteName, gameType, vpip, pfr, limp,
  facedRaise, foldToRaise, callRaise, threeBet, couldSqueeze, squeeze,
  facedThreeBet, foldToThreeBet, callThreeBet, fourBet
)
VALUES (
  '?handId', '?playerName', '?siteName', '?gameType', ?vpip, ?pfr, ?limp,
  ?facedRaise, ?foldToRaise, ?callRaise, ?threeBet, ?couldSqueeze, ?squeeze,
  ?facedThreeBet, ?foldToThreeBet, ?callThreeBet, ?fourBet
);
)raw";

  /**
   *  This parameterized query retrieves the different statistics for a given player, currently
   * - Voluntary Put Money In Pot
   * - Pre Flop Raise
   * Each hand of the site where the player was seated counts once, whatever the game type.
   * param siteName
   * param playerName
   * return columns isHero, VPIP, PFR, nbHands
   */
  static constexpr std::string_view GET_STATS_BY_SITE_AND_PLAYER_NAME = R"raw(
SELECT
  p.isHero,
  100.0 * SUM(s.vpip) / COUNT(*) AS VPIP,
  100.0 * SUM(s.pfr) / COUNT(*) AS PFR,
  COUNT(*) AS nbHands
FROM Player p
JOIN HandPlayerStats s ON s.playerName = p.playerName AND s.siteName = p.siteName
WHERE
  p.playerName = '?playerName' AND
  p.siteName = '?siteName'
GROUP BY p.playerName;
)raw";


  /**
   * This parameterized query retrieves the preflop statistics of the players seated at the last
   * hand of the given table. The statistics cover the whole history of each player on that site,
   * for the game type of that table: all its tables, not only the given one.
   * A percentage is NULL when the player had no opportunity: see nbFacedRaise and nbFacedThreeBet.
   * param siteName
   * param tableName
   * return columns playerName, siteName, isHero, playerSeat, comments, VPIP, PFR, nbHands, limp,
   * nbFacedRaise, foldToRaise, callRaise, threeBet, squeeze, nbFacedThreeBet, foldToThreeBet,
   * callThreeBet, fourBet
   */
  static constexpr std::string_view GET_PREFLOP_STATS_BY_SITE_AND_TABLE_NAME = R"raw(
SELECT
  p.playerName, p.siteName, p.isHero, seated.playerSeat, p.comments,
  100.0 * SUM(s.vpip) / COUNT(*) AS VPIP,
  100.0 * SUM(s.pfr) / COUNT(*) AS PFR,
  COUNT(*) AS nbHands,
  100.0 * SUM(s.limp) / COUNT(*) AS limp,
  SUM(s.facedRaise) AS nbFacedRaise,
  100.0 * SUM(s.foldToRaise) / NULLIF(SUM(s.facedRaise), 0) AS foldToRaise,
  100.0 * SUM(s.callRaise) / NULLIF(SUM(s.facedRaise), 0) AS callRaise,
  100.0 * SUM(s.threeBet) / NULLIF(SUM(s.facedRaise), 0) AS threeBet,
  100.0 * SUM(s.squeeze) / NULLIF(SUM(s.couldSqueeze), 0) AS squeeze,
  SUM(s.facedThreeBet) AS nbFacedThreeBet,
  100.0 * SUM(s.foldToThreeBet) / NULLIF(SUM(s.facedThreeBet), 0) AS foldToThreeBet,
  100.0 * SUM(s.callThreeBet) / NULLIF(SUM(s.facedThreeBet), 0) AS callThreeBet,
  100.0 * SUM(s.fourBet) / NULLIF(SUM(s.facedThreeBet), 0) AS fourBet
FROM HandPlayer seated
JOIN HandPlayerStats currentHand ON currentHand.handId = seated.handId AND currentHand.playerName = seated.playerName
JOIN Player p ON p.playerName = seated.playerName AND p.siteName = currentHand.siteName
JOIN HandPlayerStats s ON s.playerName = p.playerName AND s.siteName = p.siteName AND s.gameType = currentHand.gameType
WHERE seated.handId = (
  SELECT handId FROM Hand WHERE tableName = '?tableName' AND siteName = '?siteName'
  ORDER BY startDate DESC LIMIT 1
)
GROUP BY p.playerName
ORDER BY seated.playerSeat;
)raw";

  static constexpr std::string_view GET_MAX_SEATS_BY_SITE_AND_TABLE_NAME = R"raw(
SELECT
  maxSeats
FROM
  Hand h
WHERE
  h.tableName = '?tableName' AND h.siteName = '?siteName'
ORDER BY h.startDate DESC limit 1
)raw";

  static constexpr std::array<std::string_view, 12> CREATE_QUERIES = {
      CREATE_SITE,
      CREATE_HAND,
      CREATE_GAME,
      CREATE_CASH_GAME,
      CREATE_TOURNAMENT,
      CREATE_CASH_GAME_HAND,
      CREATE_TOURNAMENT_HAND,
      CREATE_ACTION,
      CREATE_PLAYER,
      CREATE_HAND_PLAYER,
      CREATE_HAND_PLAYER_STATS,
      CREATE_HAND_PLAYER_STATS_INDEX};

} // namespace phud::sql
