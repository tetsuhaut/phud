#include "gui/HudElement.hpp"
#include "statistics/PlayerStatistics.hpp"
#include <algorithm> // std::ranges::find
#include <array>
#include <format>

namespace {
  using PS = PlayerStatistics;

  constexpr std::string_view NO_DATA {"-"};

  [[nodiscard]] std::string playerName(const PS& ps) { return ps.getPlayerName(); }

  template <auto GET_NUMBER>
  [[nodiscard]] std::string number(const PS& ps) {
    return std::format("{}", (ps.*GET_NUMBER)());
  }

  /**
   * @tparam GET_PERCENTAGE the statistic
   * @tparam GET_NB_OPPORTUNITIES its denominator: 0% would be misleading if it is 0
   */
  template <auto GET_PERCENTAGE, auto GET_NB_OPPORTUNITIES>
  [[nodiscard]] std::string percentage(const PS& ps) {
    if (0 == (ps.*GET_NB_OPPORTUNITIES)()) {
      return std::string(NO_DATA);
    }

    return std::format("{:.0f}", static_cast<double>((ps.*GET_PERCENTAGE)()));
  }

  template <auto GET_PERCENTAGE>
  [[nodiscard]] std::string ofHands(const PS& ps) {
    return percentage<GET_PERCENTAGE, &PS::getNbHands>(ps);
  }

  template <auto GET_PERCENTAGE>
  [[nodiscard]] std::string ofFacedRaises(const PS& ps) {
    return percentage<GET_PERCENTAGE, &PS::getNbFacedRaise>(ps);
  }

  template <auto GET_PERCENTAGE>
  [[nodiscard]] std::string ofFacedThreeBets(const PS& ps) {
    return percentage<GET_PERCENTAGE, &PS::getNbFacedThreeBet>(ps);
  }

  constexpr auto ELEMENTS = std::to_array<HudElement>({
      // the player
      {"playerName", "Name", "Player name", "The name of the player.", &playerName, 5},
      {"nbHands", "Hands", "Number of hands",
       "The number of hands known on this player, in the current kind of game. The statistics are "
       "reliable only if it is high enough.",
       &number<&PS::getNbHands>, 2},

      // preflop
      {"vpip", "VPIP", "VPIP",
       "Voluntarily Put money In Pot: the percentage of hands where the player calls or raises "
       "preflop, without counting the blinds. A high value means a loose player.",
       &ofHands<&PS::getVoluntaryPutMoneyInPot>},
      {"pfr", "PFR", "PFR",
       "PreFlop Raise: the percentage of hands where the player raises preflop. It is close to the "
       "VPIP for an aggressive player, far from it for a passive one.",
       &ofHands<&PS::getPreFlopRaise>},
      {"af", "AF", "Aggression factor",
       "(bets + raises) / calls after the flop. Under 1 the player is passive, above 3 the player "
       "is aggressive."},
      {"limp", "Lmp", "Limp",
       "The percentage of hands where the player is the first to enter the pot, by calling the big "
       "blind.",
       &ofHands<&PS::getLimp>},
      {"limpOrCallRaise", "L/C", "Limp or call a raise",
       "The percentage of hands where the player calls the big blind or a raise preflop."},
      {"nbFacedRaise", "nR", "Faced raises",
       "The number of hands where the player faced one raise preflop. It is the sample size of "
       "fold to raise, call raise and 3-bet.",
       &number<&PS::getNbFacedRaise>, 2},
      {"foldToRaise", "FtR", "Fold to raise",
       "The percentage of folds when facing one raise preflop.",
       &ofFacedRaises<&PS::getFoldToPreFlopRaise>},
      {"callRaise", "CR", "Call raise", "The percentage of calls when facing one raise preflop.",
       &ofFacedRaises<&PS::getCallPreFlopRaise>},
      {"threeBet", "3B", "3-bet", "The percentage of reraises when facing one raise preflop.",
       &ofFacedRaises<&PS::getPreflopThreeBet>},
      {"squeeze", "Sqz", "Squeeze",
       "The percentage of reraises when facing one raise which has been called.",
       &ofFacedRaises<&PS::getSqueeze>},
      {"nbFacedThreeBet", "n3B", "Faced 3-bets",
       "The number of hands where the player raised preflop then has been reraised. It is the "
       "sample size of fold to 3-bet, call 3-bet and 4-bet.",
       &number<&PS::getNbFacedThreeBet>, 2},
      {"foldToThreeBet", "Ft3", "Fold to 3-bet",
       "The percentage of folds when the player raised then has been reraised.",
       &ofFacedThreeBets<&PS::getFoldToThreeBet>},
      {"callThreeBet", "C3B", "Call 3-bet",
       "The percentage of calls when the player raised then has been reraised.",
       &ofFacedThreeBets<&PS::getCallThreeBet>},
      {"fourBet", "4B", "4-bet",
       "The percentage of reraises when the player raised then has been reraised.",
       &ofFacedThreeBets<&PS::getFourBet>},

      // showdown
      {"wentToShowdown", "WTS", "Went to showdown",
       "The percentage of hands where the player sees the flop and goes to showdown."},

      // flop
      {"flopCbet", "FCB", "Flop c-bet",
       "The percentage of flops where the preflop raiser bets again."},
      {"foldToFlopCbet", "FFC", "Fold to flop c-bet",
       "The percentage of folds when facing a continuation bet on the flop."},
      {"callFlopCbet", "CFC", "Call flop c-bet",
       "The percentage of calls when facing a continuation bet on the flop."},
      {"raiseFlopCbet", "RFC", "Raise flop c-bet",
       "The percentage of raises when facing a continuation bet on the flop."},
      {"foldToRaiseFlopCbet", "FRF", "Fold to raise after flop c-bet",
       "The percentage of folds when the player's continuation bet on the flop is raised."},

      // turn
      {"turnCbet", "TCB", "Turn c-bet",
       "The percentage of turns where the flop bettor bets again."},
      {"foldToTurnCbet", "FTC", "Fold to turn c-bet",
       "The percentage of folds when facing a continuation bet on the turn."},
      {"callTurnCbet", "CTC", "Call turn c-bet",
       "The percentage of calls when facing a continuation bet on the turn."},
      {"raiseTurnCbet", "RTC", "Raise turn c-bet",
       "The percentage of raises when facing a continuation bet on the turn."},
      {"foldToRaiseTurnCbet", "FRT", "Fold to raise after turn c-bet",
       "The percentage of folds when the player's continuation bet on the turn is raised."},

      // river
      {"riverCbet", "RCB", "River c-bet",
       "The percentage of rivers where the turn bettor bets again."},
      {"foldToRiverCbet", "FRC", "Fold to river c-bet",
       "The percentage of folds when facing a continuation bet on the river."},
      {"callRiverCbet", "CRC", "Call river c-bet",
       "The percentage of calls when facing a continuation bet on the river."},
      {"raiseRiverCbet", "RRC", "Raise river c-bet",
       "The percentage of raises when facing a continuation bet on the river."},
      {"foldToRaiseRiverCbet", "FRR", "Fold to raise after river c-bet",
       "The percentage of folds when the player's continuation bet on the river is raised."},

      // donk bet
      {"donkBet", "DB", "Donk bet",
       "The percentage of flops where the player bets out of position into the preflop raiser."},
      {"foldToDonkBet", "FDB", "Fold to donk bet",
       "The percentage of folds of the preflop raiser when facing a donk bet."},
      {"callDonkBet", "CDB", "Call donk bet",
       "The percentage of calls of the preflop raiser when facing a donk bet."},
      {"raiseDonkBet", "RDB", "Raise donk bet",
       "The percentage of raises of the preflop raiser when facing a donk bet."},
      {"foldToRaiseDonkBet", "FRD", "Fold to raise after donk bet",
       "The percentage of folds when the player's donk bet is raised."},

      // check raise
      {"flopCheckRaise", "CkR", "Flop check-raise",
       "The percentage of flops where the player checks then raises."},
      {"callFlopCheckRaise", "CCR", "Call flop check-raise",
       "The percentage of calls when facing a check-raise on the flop."},
      {"foldToFlopCheckRaise", "FCR", "Fold to flop check-raise",
       "The percentage of folds when facing a check-raise on the flop."},
  });
} // anonymous namespace

std::span<const HudElement> getHudElements() noexcept {
  return ELEMENTS;
}

const HudElement* findHudElement(std::string_view id) noexcept {
  const auto it = std::ranges::find(ELEMENTS, id, &HudElement::id);
  return (ELEMENTS.end() == it) ? nullptr : &*it;
}

std::string formatHudElement(const HudElement& element, const PlayerStatistics& ps) {
  return hasData(element) ? element.format(ps) : std::string(NO_DATA);
}
