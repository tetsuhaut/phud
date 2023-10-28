#include "containers/algorithms.hpp" // phud::algorithms
#include "entities/Action.hpp" // ActionType, String
#include "language/assert.hpp" // phudAssert
#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;

// Note : must use frozen::string for map keys.
// frozen::string can be created from std::string_view.

static constexpr auto ACTION_TYPE_TO_STRING {
  frozen::make_unordered_map<ActionType, std::string_view>({
    { ActionType::bet, "bet" }, { ActionType::call, "call" },
    { ActionType::check, "check" }, { ActionType::fold, "fold" },
    { ActionType::raise, "raise" }, { ActionType::none, "none" }
  })
};

static constexpr auto STREET_TO_STRING {
  frozen::make_unordered_map<Street, std::string_view>({
    { Street::preflop, "preflop" }, { Street::flop, "flop" },
    { Street::turn, "turn" }, { Street::river, "river" }
  })
};

Action::Action(const Params& p)
  : m_handId { p.handId },
    m_playerName { p.playerName },
    m_street { p.street },
    m_type { p.type },
    m_index { p.actionIndex },
    m_betAmount { p.betAmount } {
  phudAssert((Street::none != m_street), "Cannot create 'none' action");
  phudAssert(!m_handId.empty(), "empty hand id");
  phudAssert(!m_playerName.empty(), "empty playerName");
  phudAssert(m_betAmount >= 0, "negative bet amount");
}

Action::~Action() = default;

std::string_view toString(ActionType at) {
  return pa::getValueFromKey(ACTION_TYPE_TO_STRING, at);
}

std::string_view toString(Street st) {
  return pa::getValueFromKey(STREET_TO_STRING, st);
}
