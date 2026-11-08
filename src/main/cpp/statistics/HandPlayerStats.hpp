#pragma once

#include <string>
#include <vector>

// forward declaration
class Hand;

/**
 * What a player did during one hand. Computed once when the hand is saved, so that reading the
 * statistics of a player is a simple sum over its hands.
 * The 'faced' and 'could' fields are the opportunities: the denominators of the statistics.
 * statistic = nb of action done / nb of action opportunities
 */
struct [[nodiscard]] HandPlayerStats final {
  std::string playerName;
  bool vpip {false};
  bool pfr {false};
  bool limp {false};
  bool facedRaise {false}; // facing one raise, the player had not raised yet
  bool foldToRaise {false};
  bool callRaise {false};
  bool threeBet {false};
  bool couldSqueeze {false}; // facing one raise which has been called
  bool squeeze {false};
  bool facedThreeBet {false}; // the player raised, and has been raised
  bool foldToThreeBet {false};
  bool callThreeBet {false};
  bool fourBet {false};
};

/**
 * @returns one element for each player seated at the given hand, including those who did not act.
 */
[[nodiscard]] std::vector<HandPlayerStats> computeHandPlayerStats(const Hand& hand);
