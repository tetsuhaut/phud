#include "entities/Action.hpp"
#include "entities/Hand.hpp" // std::array
#include "statistics/HandPlayerStats.hpp"
#include <algorithm> // std::ranges::find_if, std::ranges::transform
#include <iterator>  // std::back_inserter
#include <ranges>    // std::views::filter

namespace {
  /**
   * The stats of a player being computed, with what we need to know about its previous actions.
   */
  struct [[nodiscard]] PlayerProgress final {
    HandPlayerStats stats;
    int nbRaises {0};
    bool hasActed {false};
  };

  /**
   * What happened at the table before the current action.
   */
  struct [[nodiscard]] TableProgress final {
    int nbRaises {0};
    int nbCallersOfRaise {0};
  };

  [[nodiscard]] std::vector<PlayerProgress> newPlayerProgresses(const Hand& hand) {
    std::vector<PlayerProgress> ret;

    for (const auto& name : hand.getSeats()) {
      if (!name.empty()) {
        ret.push_back({.stats = {.playerName = name}});
      }
    }

    return ret;
  }

  void updateWhenFacingRaise(HandPlayerStats& stats, ActionType type, const TableProgress& table) {
    const auto isRaise = ActionType::raise == type;
    stats.facedRaise = true;
    stats.foldToRaise = stats.foldToRaise or ActionType::fold == type;
    stats.callRaise = stats.callRaise or ActionType::call == type;
    stats.threeBet = stats.threeBet or isRaise;

    if (0 < table.nbCallersOfRaise) {
      stats.couldSqueeze = true;
      stats.squeeze = stats.squeeze or isRaise;
    }
  }

  void updateWhenFacingThreeBet(HandPlayerStats& stats, ActionType type) {
    stats.facedThreeBet = true;
    stats.foldToThreeBet = stats.foldToThreeBet or ActionType::fold == type;
    stats.callThreeBet = stats.callThreeBet or ActionType::call == type;
    stats.fourBet = stats.fourBet or ActionType::raise == type;
  }

  void updatePlayer(PlayerProgress& player, ActionType type, const TableProgress& table) {
    auto& stats = player.stats;
    const auto isCall = ActionType::call == type;
    const auto isRaise = ActionType::raise == type;
    stats.vpip = stats.vpip or isCall or isRaise or ActionType::bet == type;
    stats.pfr = stats.pfr or isRaise;
    stats.limp = stats.limp or (0 == table.nbRaises and !player.hasActed and isCall);

    if (1 == table.nbRaises and 0 == player.nbRaises) {
      updateWhenFacingRaise(stats, type, table);
    }

    if (2 == table.nbRaises and 1 == player.nbRaises) {
      updateWhenFacingThreeBet(stats, type);
    }

    player.nbRaises += isRaise ? 1 : 0;
    player.hasActed = true;
  }

  void updateTable(TableProgress& table, ActionType type) {
    if (ActionType::call == type and 1 == table.nbRaises) {
      table.nbCallersOfRaise++;
    }

    if (ActionType::raise == type) {
      table.nbRaises++;
    }
  }

  [[nodiscard]] bool isPreflopDecision(const Action* pAction) {
    return Street::preflop == pAction->getStreet() and ActionType::win != pAction->getType();
  }
} // anonymous namespace

std::vector<HandPlayerStats> computeHandPlayerStats(const Hand& hand) {
  auto players = newPlayerProgresses(hand);
  TableProgress table;
  const auto actions = hand.viewActions();

  for (const auto* pAction : actions | std::views::filter(isPreflopDecision)) {
    const auto type = pAction->getType();
    const auto it = std::ranges::find_if(players, [pAction](const auto& p) {
      return pAction->getPlayerName() == p.stats.playerName;
    });

    if (players.end() != it) {
      updatePlayer(*it, type, table);
    }

    updateTable(table, type);
  }

  std::vector<HandPlayerStats> ret;
  ret.reserve(players.size());
  std::ranges::transform(players, std::back_inserter(ret),
                         [](auto& p) { return std::move(p.stats); });
  return ret;
}
