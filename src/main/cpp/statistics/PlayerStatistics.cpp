#include "statistics/PlayerStatistics.hpp" // PlayerStatistics, std::string_view, std::string, PlayerStatistics::Params

PlayerStatistics::PlayerStatistics(const Params& p) noexcept
  : m_nbHands { p.nbHands },
m_playerName { p.playerName },
m_siteName { p.siteName },
m_isHero { p.isHero },
m_voluntaryPutMoneyInPot { p.vpip },
m_preflopRaise { p.pfr } {}

PlayerStatistics::~PlayerStatistics() = default;


//PlayerStatistics::PlayerStatistics(PlayerStatistics&& p) :
//  m_nbHands{ p.m_nbHands },
//  m_playerName{ p.m_playerName },
//m_siteName { p.m_siteName },
//m_isHero { p.m_isHero },
//m_voluntaryPutMoneyInPot { p.m_voluntaryPutMoneyInPot },
//m_preflopRaise { p.m_preflopRaise },
//m_aggressionFactor { p.m_aggressionFactor },
//m_limpOrCallPreFlopRaise { p.m_limpOrCallPreFlopRaise },
//m_foldToPreFlopRaise { p.m_foldToPreFlopRaise },
//m_preFlopThreeBet { p.m_preFlopThreeBet },
//m_foldToPreFlopThreeBet { p.m_foldToPreFlopThreeBet },
//m_callPreFlopThreeBet { p.m_callPreFlopThreeBet },
//m_wentToShowDown { p.m_wentToShowDown },
//m_flopContinuationBet { p.m_flopContinuationBet },
//m_foldToFlopContinuationBet { p.m_foldToFlopContinuationBet },
//m_callFlopContinuationBet { p.m_callFlopContinuationBet },
//m_raiseAfterFlopContinuationBet { p.m_raiseAfterFlopContinuationBet },
//m_foldToRaiseAfterFlopContinuationBet { p.m_foldToRaiseAfterFlopContinuationBet },
//m_turnContinuationBet { p.m_turnContinuationBet },
//m_foldToTurnContinuationBet { p.m_foldToTurnContinuationBet },
//m_callTurnContinuationBet { p.m_callTurnContinuationBet },
//m_raiseAfterTurnContinuationBet { p.m_raiseAfterTurnContinuationBet },
//m_foldToRaiseAfterTurnContinuationBet { p.m_foldToRaiseAfterTurnContinuationBet },
//m_riverContinuationBet { p.m_riverContinuationBet },
//m_foldToRiverContinuationBet { p.m_foldToRiverContinuationBet },
//m_callRiverContinuationBet { p.m_callRiverContinuationBet },
//m_raiseAfterRiverContinuationBet { p.m_raiseAfterRiverContinuationBet },
//m_foldToRaiseAfterRiverContinuationBet { p.m_foldToRaiseAfterRiverContinuationBet },
//m_donkBet { p.m_donkBet },
//m_foldToDonkBet { p.m_foldToDonkBet },
//m_callDonkBet { p.m_callDonkBet },
//m_raiseAfterDonkBet { p.m_raiseAfterDonkBet },
//m_foldToRaiseAfterDonkBet { p.m_foldToRaiseAfterDonkBet },
//m_flopCheckRaise { p.m_flopCheckRaise },
//m_callFlopCheckRaise { p.m_callFlopCheckRaise },
//m_foldToFlopCheckRaise{ p.m_foldToFlopCheckRaise }
//{}

//PlayerStatistics::PlayerStatistics(const PlayerStatistics& p) :
//m_nbHands {
//  p.m_nbHands
//},
//m_playerName{ p.m_playerName },
//m_siteName{ p.m_siteName },
//m_isHero{ p.m_isHero },
//m_voluntaryPutMoneyInPot{ p.m_voluntaryPutMoneyInPot },
//m_preflopRaise{ p.m_preflopRaise },
//m_aggressionFactor{ p.m_aggressionFactor },
//m_limpOrCallPreFlopRaise{ p.m_limpOrCallPreFlopRaise },
//m_foldToPreFlopRaise{ p.m_foldToPreFlopRaise },
//m_preFlopThreeBet{ p.m_preFlopThreeBet },
//m_foldToPreFlopThreeBet{ p.m_foldToPreFlopThreeBet },
//m_callPreFlopThreeBet{ p.m_callPreFlopThreeBet },
//m_wentToShowDown{ p.m_wentToShowDown },
//m_flopContinuationBet{ p.m_flopContinuationBet },
//m_foldToFlopContinuationBet{ p.m_foldToFlopContinuationBet },
//m_callFlopContinuationBet{ p.m_callFlopContinuationBet },
//m_raiseAfterFlopContinuationBet{ p.m_raiseAfterFlopContinuationBet },
//m_foldToRaiseAfterFlopContinuationBet{ p.m_foldToRaiseAfterFlopContinuationBet },
//m_turnContinuationBet{ p.m_turnContinuationBet },
//m_foldToTurnContinuationBet{ p.m_foldToTurnContinuationBet },
//m_callTurnContinuationBet{ p.m_callTurnContinuationBet },
//m_raiseAfterTurnContinuationBet{ p.m_raiseAfterTurnContinuationBet },
//m_foldToRaiseAfterTurnContinuationBet{ p.m_foldToRaiseAfterTurnContinuationBet },
//m_riverContinuationBet{ p.m_riverContinuationBet },
//m_foldToRiverContinuationBet{ p.m_foldToRiverContinuationBet },
//m_callRiverContinuationBet{ p.m_callRiverContinuationBet },
//m_raiseAfterRiverContinuationBet{ p.m_raiseAfterRiverContinuationBet },
//m_foldToRaiseAfterRiverContinuationBet{ p.m_foldToRaiseAfterRiverContinuationBet },
//m_donkBet{ p.m_donkBet },
//m_foldToDonkBet{ p.m_foldToDonkBet },
//m_callDonkBet{ p.m_callDonkBet },
//m_raiseAfterDonkBet{ p.m_raiseAfterDonkBet },
//m_foldToRaiseAfterDonkBet{ p.m_foldToRaiseAfterDonkBet },
//m_flopCheckRaise{ p.m_flopCheckRaise },
//m_callFlopCheckRaise{ p.m_callFlopCheckRaise },
//m_foldToFlopCheckRaise{ p.m_foldToFlopCheckRaise }
//{}

