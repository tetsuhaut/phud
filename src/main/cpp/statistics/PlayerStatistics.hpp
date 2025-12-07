#pragma once

#include "language/limits.hpp" // toInt
#include <string>
#include <string_view>

// cf. https://www.pokertracker.com/guides/PT3/general/statistical-reference-guide
// VPIP = Voluntary Put money In Pot = (preflop calls + preflop bets + preflop raises) / (preflop
// calls + preflop bet + preflop raises + preflop checks + preflop folds) PFR = PreFlop Raise =
// (preflop bets + preflop raises) / (preflop calls + preflop bet + preflop raises + preflop checks
// + preflop folds) Agression factor = (bets + raises) / calls

// must be copiable to be used in another thread
class [[nodiscard]] PlayerStatistics final {
private:
  // Memory layout optimized: largest to smallest to minimize padding
  // strings first (typically 32 bytes with SSO on MSVC x64)
  std::string m_playerName;
  std::string m_siteName;

  // All doubles together (8 bytes each, alignment 8)
  double m_voluntaryPutMoneyInPot;
  double m_preflopRaise;
  double m_aggressionFactor {0};
  double m_limpOrCallPreFlopRaise {0};
  double m_foldToPreFlopRaise {0};
  double m_preFlopThreeBet {0};
  double m_foldToPreFlopThreeBet {0};
  double m_callPreFlopThreeBet {0};
  double m_wentToShowDown {0};
  double m_flopContinuationBet {0};
  double m_foldToFlopContinuationBet {0};
  double m_callFlopContinuationBet {0};
  double m_raiseAfterFlopContinuationBet {0};
  double m_foldToRaiseAfterFlopContinuationBet {0};
  double m_turnContinuationBet {0};
  double m_foldToTurnContinuationBet {0};
  double m_callTurnContinuationBet {0};
  double m_raiseAfterTurnContinuationBet {0};
  double m_foldToRaiseAfterTurnContinuationBet {0};
  double m_riverContinuationBet {0};
  double m_foldToRiverContinuationBet {0};
  double m_callRiverContinuationBet {0};
  double m_raiseAfterRiverContinuationBet {0};
  double m_foldToRaiseAfterRiverContinuationBet {0};
  double m_donkBet {0};
  double m_foldToDonkBet {0};
  double m_callDonkBet {0};
  double m_raiseAfterDonkBet {0};
  double m_foldToRaiseAfterDonkBet {0};
  double m_flopCheckRaise {0};
  double m_callFlopCheckRaise {0};
  double m_foldToFlopCheckRaise {0};

  // int (4 bytes, alignment 4)
  int m_nbHands;

  // bool last (1 byte, alignment 1) - minimizes total padding
  bool m_isHero;

public:
  struct [[nodiscard]] Params final {
    std::string_view playerName;
    std::string_view siteName;
    bool isHero;
    int nbHands;
    double vpip;
    double pfr;
  };

  explicit PlayerStatistics(const Params& p) noexcept;
  PlayerStatistics(const PlayerStatistics&) = delete;
  PlayerStatistics(PlayerStatistics&&) = delete;
  PlayerStatistics& operator=(const PlayerStatistics&) = delete;
  PlayerStatistics& operator=(PlayerStatistics&&) = delete;
  ~PlayerStatistics();
  [[nodiscard]] constexpr const std::string& getSiteName() const noexcept { return m_siteName; }
  [[nodiscard]] constexpr const std::string& getPlayerName() const noexcept { return m_playerName; }
  [[nodiscard]] constexpr bool isHero() const noexcept { return m_isHero; }
  [[nodiscard]] constexpr int getNbHands() const noexcept { return m_nbHands; }

  /** @return The percentage of hands the player puts money into the pot preflop, without counting
   * any blind postings.
   */
  [[nodiscard]] constexpr int getVoluntaryPutMoneyInPot() const noexcept {
    return limits::toInt(m_voluntaryPutMoneyInPot);
  }

  /** @return The percentage of hands the player (re)raises preflop.
   * note: pfr <= vpip
   * note: VPIP - PFR > 5 -> player is too aggressive preflop
   * note: VPIP - PFR > 10 -> player is really too aggressive preflop
   */
  [[nodiscard]] constexpr int getPreFlopRaise() const noexcept {
    return limits::toInt(m_preflopRaise);
  }

  // between 0 (passive) and 4 (aggressive)
  [[nodiscard]] constexpr double getAggressionFactor() const noexcept { return m_aggressionFactor; }

  /** @return The percentage of hands the player called the big blind or the raise preflop.
   */
  [[nodiscard]] constexpr double getLimpOrCallPreFlopRaise() const noexcept {
    return m_limpOrCallPreFlopRaise;
  }

  [[nodiscard]] constexpr double getFoldToPreFlopRaise() const noexcept {
    return m_foldToPreFlopRaise;
  }
  [[nodiscard]] constexpr double getPreflopThreeBet() const noexcept { return m_preFlopThreeBet; }
  [[nodiscard]] constexpr double getFoldToThreeBet() const noexcept {
    return m_foldToPreFlopThreeBet;
  }
  [[nodiscard]] constexpr double getCallThreeBet() const noexcept { return m_callPreFlopThreeBet; }
  [[nodiscard]] constexpr double getWentToShowDown() const noexcept { return m_wentToShowDown; }
  [[nodiscard]] constexpr double getFlopContinuationBet() const noexcept {
    return m_flopContinuationBet;
  }
  [[nodiscard]] constexpr double getFoldToFlopContinuationBet() const noexcept {
    return m_foldToFlopContinuationBet;
  }
  [[nodiscard]] constexpr double getCallFlopContinuationBet() const noexcept {
    return m_callFlopContinuationBet;
  }
  [[nodiscard]] constexpr double getRaiseAfterFlopContinuationBet() const noexcept {
    return m_raiseAfterFlopContinuationBet;
  }
  [[nodiscard]] constexpr double getFoldToRaiseAfterFlopContinuationBet() const noexcept {
    return m_foldToRaiseAfterFlopContinuationBet;
  }
  [[nodiscard]] constexpr double getTurnContinuationBet() const noexcept {
    return m_turnContinuationBet;
  }
  [[nodiscard]] constexpr double getFoldToTurnContinuationBet() const noexcept {
    return m_foldToTurnContinuationBet;
  }
  [[nodiscard]] constexpr double getCallTurnContinuationBet() const noexcept {
    return m_callTurnContinuationBet;
  }
  [[nodiscard]] constexpr double getRaiseAfterTurnContinuationBet() const noexcept {
    return m_raiseAfterTurnContinuationBet;
  }
  [[nodiscard]] constexpr double getFoldToRaiseAfterTurnContinuationBet() const noexcept {
    return m_foldToRaiseAfterTurnContinuationBet;
  }
  [[nodiscard]] constexpr double getRiverContinuationBet() const noexcept {
    return m_riverContinuationBet;
  }
  [[nodiscard]] constexpr double getFoldToRiverContinuationBet() const noexcept {
    return m_foldToRiverContinuationBet;
  }
  [[nodiscard]] constexpr double getCallRiverContinuationBet() const noexcept {
    return m_callRiverContinuationBet;
  }
  [[nodiscard]] constexpr double getRaiseAfterRiverContinuationBet() const noexcept {
    return m_raiseAfterRiverContinuationBet;
  }
  [[nodiscard]] constexpr double getFoldToRaiseAfterRiverContinuationBet() const noexcept {
    return m_foldToRaiseAfterRiverContinuationBet;
  }
  [[nodiscard]] constexpr double getDonkBet() const noexcept { return m_donkBet; }
  [[nodiscard]] constexpr double getFoldToDonkBet() const noexcept { return m_foldToDonkBet; }
  [[nodiscard]] constexpr double getCallDonkBet() const noexcept { return m_callDonkBet; }
  [[nodiscard]] constexpr double getRaiseAfterDonkBet() const noexcept {
    return m_raiseAfterDonkBet;
  }
  [[nodiscard]] constexpr double getFoldToRaiseAfterDonkBet() const noexcept {
    return m_foldToRaiseAfterDonkBet;
  }
  [[nodiscard]] constexpr double getFlopCheckRaise() const noexcept { return m_flopCheckRaise; }
  [[nodiscard]] constexpr double getCallFlopCheckRaise() const noexcept {
    return m_callFlopCheckRaise;
  }
  [[nodiscard]] constexpr double getFoldToFlopCheckRaise() const noexcept {
    return m_foldToFlopCheckRaise;
  }
}; // class PlayerStatistics
