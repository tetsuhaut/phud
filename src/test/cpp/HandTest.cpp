#include "TestInfrastructure.hpp"
#include "entities/Action.hpp" // ActionType, Street
#include "entities/Card.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp" // Time
#include "entities/Player.hpp" // needed as Site declares incomplete Player type
#include "entities/Site.hpp"
#include "filesystem/TextFile.hpp"
#include "history/GameData.hpp" // std::string, std::string_view, Time, Variant, Limit
#include "history/WinamaxHandBuilder.hpp" // toAmount, toBuyIn
#include "history/WinamaxGameHistory.hpp"
#include "mainLib/ProgramInfos.hpp"
#include "threads/PlayerCache.hpp"

#include <utf8.h> // utf8::utf16to8
#include <optional>

namespace ps = phud::strings;
namespace pt = phud::test;

/**
 * exported for unit testing
 * @returns isRealMoney, gameName, variant, limit
 */
struct FileStem {
  bool m_isRealMoney;
  std::string m_gameName;
  Variant m_variant;
  Limit m_limit;
};
[[nodiscard]] std::optional<FileStem> parseFileStem(std::string_view fileStem);

static inline std::string utf16To8(std::wstring_view utf16String) {
  std::string ret;
  utf8::utf16to8(std::begin(utf16String), std::end(utf16String), std::back_inserter(ret));
  return ret;
}

BOOST_AUTO_TEST_SUITE(HandTest)

BOOST_AUTO_TEST_CASE(HandTest_loadingTournamentHandWithoutActionShouldSucceed) {
  const auto& file { pt::getFileFromTestResources("Winamax/hands/20150718_holdemTournament(123703551)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  const auto& tournaments { pSite->viewTournaments() };
  BOOST_REQUIRE(1 == tournaments.size());
  const auto& t { tournaments.front() };
  BOOST_REQUIRE(1 == t->viewHands().size());
  const auto pHand { t->viewHands().front() };
  BOOST_REQUIRE(5 == pHand->viewActions().size());
  BOOST_REQUIRE(Street::preflop == pHand->viewActions().front()->getStreet());
  BOOST_REQUIRE(ActionType::raise == pHand->viewActions().front()->getType());
  BOOST_REQUIRE("sabre_laser" == pHand->viewActions().front()->getPlayerName());
  std::size_t i { 0 };
  std::ranges::for_each(pHand->viewActions(), [&i](const auto pAction) {
    BOOST_REQUIRE(i == pAction->getIndex());
    BOOST_REQUIRE("531302705944068104-65-1437230557" == pAction->getHandId());
    i++;
  });
  BOOST_REQUIRE(3200 == pHand->viewActions().front()->getBetAmount());
  BOOST_REQUIRE(5 == pHand->viewActions().size());
}

BOOST_AUTO_TEST_CASE(HandTest_loadingDoubleOrNothingWithOnlyFoldsShouldSucceed) {
  const auto& file { pt::getFileFromTestResources("Winamax/hands/20141031_Double or Nothing(98932321)_real_holdem_no-limit.txt") };
  const auto& pSite { WinamaxGameHistory::parseGameHistory(file) };
  BOOST_REQUIRE(nullptr != pSite);
  BOOST_REQUIRE(pSite->viewCashGames().empty());
  const auto& tournaments { pSite->viewTournaments() };
  BOOST_REQUIRE(1 == tournaments.size());
  const auto& t { tournaments.front() };
  BOOST_REQUIRE("Double or Nothing(98932321)" == t->getName());
  BOOST_REQUIRE(t->isRealMoney());
  BOOST_REQUIRE_MESSAGE(Time({ .strTime = "2014/10/31 00:45:01", .format = WINAMAX_HISTORY_TIME_FORMAT })
                        == t->getStartDate(),
                        "startDate='" << t->getStartDate().toSqliteDate() << '\'');
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == t->getSiteName());
  BOOST_REQUIRE_MESSAGE("20141031_Double or Nothing(98932321)_real_holdem_no-limit" == t->getId(),
                        "wrong id, t->getId() '" << t->getId() << '\'');
  BOOST_REQUIRE(Variant::holdem == t->getVariant());
  BOOST_REQUIRE(Limit::noLimit == t->getLimitType());
  BOOST_REQUIRE(Seat::seatTen == t->getMaxNbSeats());
  BOOST_REQUIRE(2.0 == t->getBuyIn());
  BOOST_REQUIRE(1 == t->viewHands().size());
  const auto& h { *t->viewHands().front() };
  BOOST_REQUIRE(10 == h.viewActions().size());
  BOOST_REQUIRE("424911083212374017-1-1414716301" == h.getId());
  BOOST_REQUIRE(GameType::tournament == h.getGameType());
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == h.getSiteName());
  BOOST_REQUIRE("Double or Nothing(98932321)#0" == h.getTableName());
  BOOST_REQUIRE("2014-10-31 00:45:01" == h.getStartDate().toSqliteDate());
  BOOST_REQUIRE("Bordel92" == h.getSeats().front());
  BOOST_REQUIRE("Illuminatiz" == h.getSeats()[1]);
  BOOST_REQUIRE("sabre_laser" == h.getSeats()[2]);
  BOOST_REQUIRE("el fouAArat" == h.getSeats()[3]);
  BOOST_REQUIRE("DEEPstef" == h.getSeats()[4]);
  BOOST_REQUIRE("crazypix" == h.getSeats()[5]);
  BOOST_REQUIRE("blackviz" == h.getSeats()[6]);
  BOOST_REQUIRE("romjeresci" == h.getSeats()[7]);
  BOOST_REQUIRE("Smaky98" == h.getSeats()[8]);
  BOOST_REQUIRE_MESSAGE("djembe man" == h.getSeats()[9],
                        "wrong player name, h.getSeats()[9]='" << h.getSeats()[9] << '\'');
  BOOST_REQUIRE(Seat::seatOne == h.getButtonSeat());
  BOOST_REQUIRE(0.0 == h.getAnte());
  BOOST_REQUIRE(0 == h.getLevel());
  BOOST_REQUIRE(h.isPlayerInvolved("Bordel92"));
  BOOST_REQUIRE(h.isPlayerInvolved("Illuminatiz"));
  BOOST_REQUIRE(h.isPlayerInvolved("sabre_laser"));
  BOOST_REQUIRE(h.isPlayerInvolved("el fouAArat"));
  BOOST_REQUIRE(h.isPlayerInvolved("DEEPstef"));
  BOOST_REQUIRE(h.isPlayerInvolved("crazypix"));
  BOOST_REQUIRE(h.isPlayerInvolved("blackviz"));
  BOOST_REQUIRE(h.isPlayerInvolved("romjeresci"));
  BOOST_REQUIRE(h.isPlayerInvolved("Smaky98"));
  BOOST_REQUIRE(h.isPlayerInvolved("djembe man"));
  BOOST_REQUIRE(!h.isPlayerInvolved("toto"));
  BOOST_REQUIRE(Card::tenHeart == h.getHeroCard1());
  BOOST_REQUIRE(Card::fiveDiamond == h.getHeroCard2());
  BOOST_REQUIRE(Card::none == h.getHeroCard3());
  BOOST_REQUIRE(Card::none == h.getHeroCard4());
  BOOST_REQUIRE(Card::none == h.getHeroCard5());
  BOOST_REQUIRE(Card::none == h.getBoardCard1());
  BOOST_REQUIRE(Card::none == h.getBoardCard2());
  BOOST_REQUIRE(Card::none == h.getBoardCard3());
  BOOST_REQUIRE(Card::none == h.getBoardCard4());
  BOOST_REQUIRE(Card::none == h.getBoardCard5());
  BOOST_REQUIRE(!h.isWinner("Bordel92"));
  BOOST_REQUIRE(!h.isWinner("Illuminatiz"));
  BOOST_REQUIRE(!h.isWinner("sabre_laser"));
  BOOST_REQUIRE(!h.isWinner("el fouAArat"));
  BOOST_REQUIRE(h.isWinner("DEEPstef"));
  BOOST_REQUIRE(!h.isWinner("crazypix"));
  BOOST_REQUIRE(!h.isWinner("blackviz"));
  BOOST_REQUIRE(!h.isWinner("romjeresci"));
  BOOST_REQUIRE(!h.isWinner("Smaky98"));
  BOOST_REQUIRE(!h.isWinner("djembe man"));
  BOOST_REQUIRE(!h.isWinner("toto"));
}

BOOST_AUTO_TEST_CASE(HandTest_parsingDoubleOrNothingTournamentFileNameShouldSuceed) {
  const auto& oTuple { parseFileStem("20141116_Double or Nothing(100679030)_real_holdem_no-limit") };
  BOOST_REQUIRE(oTuple.has_value());
  const auto [isRealMoney, gameName, variant, limit] { oTuple.value() };
  BOOST_REQUIRE("Double or Nothing(100679030)" == gameName);
  BOOST_REQUIRE(true == isRealMoney);
  BOOST_REQUIRE(Variant::holdem == variant);
  BOOST_REQUIRE(Limit::noLimit == limit);
}

BOOST_AUTO_TEST_CASE(HandTest_parsingSuperFreeRollTournamentFileNameShouldSucceed) {
  const auto [isRealMoney, gameName, variant, limit] {
    parseFileStem(utf16To8(L"20150630_Super Freeroll Stade 1 - Déglingos _"
                           "(123322389)_real_holdem_no-limit")).value()
  };
  BOOST_REQUIRE_MESSAGE(utf16To8(L"Super Freeroll Stade 1 - Déglingos _(123322389)") ==
                        gameName, "gameName=" << gameName);
  BOOST_REQUIRE(true == isRealMoney);
  BOOST_REQUIRE(Variant::holdem == variant);
  BOOST_REQUIRE(Limit::noLimit == limit);
}

BOOST_AUTO_TEST_CASE(HandTest_parsingOmaha5CashgameFileNameShouldSucceed) {
  const auto [isRealMoney, gameName, variant, limit] { parseFileStem("20180304_Ferrare 02_play_omaha5_pot-limit").value() };
  BOOST_REQUIRE("Ferrare 02_play_omaha5_pot-limit" == gameName);
  BOOST_REQUIRE(false == isRealMoney);
  BOOST_REQUIRE(Variant::omaha5 == variant);
  BOOST_REQUIRE(Limit::potLimit == limit);
}

BOOST_AUTO_TEST_CASE(HandTest_parsingShortTrackCashgameFileNameShouldSucceed) {
  const auto [isRealMoney, gameName, variant, limit] { parseFileStem(utf16To8(L"20180304_SHORT TRACK 0,10€_real_holdem_no-limit")).value() };
  BOOST_REQUIRE(utf16To8(L"SHORT TRACK 0,10€") == gameName);
  BOOST_REQUIRE(true == isRealMoney);
  BOOST_REQUIRE(Variant::holdem == variant);
  BOOST_REQUIRE(Limit::noLimit == limit);
}

BOOST_AUTO_TEST_CASE(HandTest_buildCashGameHandShouldSucceed) {
  pt::TmpFile file { "20150718_Hold'em(123703551)_play_holdem_no-limit.txt" };
  file.printLn(
    "Winamax Poker - Tournament ""Hold'em"" buyIn: 4,50 + 0,50 level: 17 - HandId: #531302705944068104-65-1437230557 - Holdem no limit (160/800/1600) - 2015/07/18 14:42:37 UTC");
  file.printLn("Table: 'Hold'em(123703551)#07' 6-max (play money) Seat #4 is the button");
  file.printLn("Seat 1: trinita34 (45650)");
  file.printLn("Seat 2: sabre_laser (17220)");
  file.printLn("Seat 3: Dukette974 (6995)");
  file.printLn("Seat 6: willbelucky (70120)");
  file.printLn("*** ANTE /BLINDS ***");
  file.printLn("willbelucky posts ante 160");
  file.printLn("trinita34 posts ante 160");
  file.printLn("sabre_laser posts ante 160");
  file.printLn("Dukette974 posts ante 160");
  file.printLn("willbelucky posts small blind 800");
  file.printLn("trinita34 posts big blind 1600");
  file.printLn("Dealt to sabre_laser [2d 2c]");
  file.printLn("*** PRE-FLOP *** ");
  file.printLn("sabre_laser raises 1600 to 3200");
  file.printLn("Dukette974 folds");
  file.printLn("willbelucky folds");
  file.printLn("trinita34 raises 42290 to 45490 and is all-in");
  file.printLn("sabre_laser calls 13860 and is all-in");
  file.printLn("*** FLOP *** [Jd Tc 5d]");
  file.printLn("*** TURN *** [Jd Tc 5d][5c]");
  file.printLn("*** RIVER *** [Jd Tc 5d 5c][Th]");
  file.printLn("*** SHOW DOWN ***");
  file.printLn("sabre_laser shows [2d 2c] (Two pairs : Tens and 5)");
  file.printLn("trinita34 shows [Kd 2s] (Two pairs : Tens and 5)");
  file.printLn("trinita34 collected 63990 from pot");
  file.printLn("*** SUMMARY ***");
  file.printLn("Total pot 63990 | No rake");
  file.printLn("Board: [Jd Tc 5d 5c Th]");
  file.printLn(
    "Seat 1: trinita34 (big blind) showed [Kd 2s] and won 63990 with Two pairs : Tens and 5");
  file.printLn("Seat 2: sabre_laser showed [2d 2c] and lost with Two pairs : Tens and 5");
  file.printLn("");
  file.printLn("");
  TextFile line { file.path() };
  line.next();
  PlayerCache _ { ProgramInfos::WINAMAX_SITE_NAME };
  const auto& hand { WinamaxHandBuilder::buildHand<Tournament>(line, _) };
  BOOST_REQUIRE(160 == hand->getAnte());
  BOOST_REQUIRE(Card::jackDiamond == hand->getBoardCard1());
  BOOST_REQUIRE(Card::tenClub == hand->getBoardCard2());
  BOOST_REQUIRE(Card::fiveDiamond == hand->getBoardCard3());
  BOOST_REQUIRE(Card::fiveClub == hand->getBoardCard4());
  BOOST_REQUIRE(Card::tenHeart == hand->getBoardCard5());
}

BOOST_AUTO_TEST_CASE(HandTest_buildTournamentHandOneRaiseBbIsAllInShouldSucceed) {
  TextFile line(pt::getFileFromTestResources(
                  "Winamax/hands/20141119_Freeroll(99427750)_real_holdem_no-limit.txt"));
  line.next();
  PlayerCache _ { ProgramInfos::WINAMAX_SITE_NAME };
  const auto& hand { WinamaxHandBuilder::buildHand<Tournament>(line, _) };
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == hand->getSiteName());
  BOOST_REQUIRE("2014-11-19 21:06:52" == hand->getStartDate().toSqliteDate());
  const auto& playerNames { hand->getSeats() };
  BOOST_REQUIRE(playerNames[0] == "kiki28700");
  BOOST_REQUIRE(playerNames[1] == "GuiGui.39");
  BOOST_REQUIRE(playerNames[2] == "sabre_laser");
  BOOST_REQUIRE(playerNames[3] == "Kermit21");
  BOOST_REQUIRE(playerNames[4] == "Sharony55");
  BOOST_REQUIRE(playerNames[5] == "M.Gee31");
  BOOST_REQUIRE(playerNames[6] == "LUKEP72");
  BOOST_REQUIRE(playerNames[7] == "eirinn33");
  BOOST_REQUIRE(playerNames[8] == "LudoSab08");
  BOOST_REQUIRE(playerNames[9] == "");
  BOOST_REQUIRE(100 == hand->getAnte());
  BOOST_REQUIRE(Card::twoClub == hand->getBoardCard1());
  BOOST_REQUIRE(Card::eightClub == hand->getBoardCard2());
  BOOST_REQUIRE(Card::nineSpade == hand->getBoardCard3());
  BOOST_REQUIRE(Card::kingSpade == hand->getBoardCard4());
  BOOST_REQUIRE(Card::aceDiamond == hand->getBoardCard5());
  BOOST_REQUIRE(Seat::seatOne == hand->getButtonSeat());
  BOOST_REQUIRE(GameType::tournament == hand->getGameType());
  BOOST_REQUIRE(Card::twoHeart == hand->getHeroCard1());
  BOOST_REQUIRE(Card::twoDiamond == hand->getHeroCard2());
  BOOST_REQUIRE(Card::none == hand->getHeroCard3());
  BOOST_REQUIRE(Card::none == hand->getHeroCard4());
  BOOST_REQUIRE(Card::none == hand->getHeroCard5());
  BOOST_REQUIRE("427038934564864214-37-1416431212" == hand->getId());
  BOOST_REQUIRE(15 == hand->getLevel());
}

BOOST_AUTO_TEST_CASE(HandTest_loadingDoubleOrNothingWith2CallersShouldSucceed) {
  TextFile line(pt::getFileFromTestResources(
                  "Winamax/hands/20150115_Double or Nothing(106820182)_real_holdem_no-limit.txt"));
  line.next();
  PlayerCache _ { ProgramInfos::WINAMAX_SITE_NAME };
  const auto& hand { WinamaxHandBuilder::buildHand<CashGame>(line, _) };
  BOOST_REQUIRE(ProgramInfos::WINAMAX_SITE_NAME == hand->getSiteName());
}

BOOST_AUTO_TEST_CASE(HandTest_playerWithNoActionHasActionNone) {
  const auto [isRealMoney, gameName, variant, limit] {
    parseFileStem("20150630_Super Freeroll Stade 1 - Déglingos _(123322389)_real_holdem_no-limit").value()
  };
  BOOST_REQUIRE_MESSAGE("Super Freeroll Stade 1 - Déglingos _(123322389)" == gameName,
                        "wrong gameName '" << gameName << "'");
  BOOST_REQUIRE(Variant::holdem == variant);
  BOOST_REQUIRE(Limit::noLimit == limit);
  BOOST_REQUIRE(true == isRealMoney);
}

struct [[nodiscard]] MyStruct1 final {
  std::unique_ptr<std::string> str;
};

std::string func(MyStruct1& s) {
  auto ret = std::move(s.str);
  return *ret;
}

BOOST_AUTO_TEST_CASE(HandTest_passingAUniquePtrShouldBeOk) {
  std::unique_ptr<std::string> myString { std::make_unique<std::string>("my string") };
  MyStruct1 param { .str = std::move(myString) };
  BOOST_REQUIRE("my string" == func(param));
}

BOOST_AUTO_TEST_SUITE_END()