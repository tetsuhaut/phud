#include "entities/Action.hpp" // ActionType, std::string
#include "language/assert.hpp" // phudAssert
#include "language/EnumMapper.hpp"

static constexpr auto ACTION_TYPE_MAPPER = makeEnumMapper<ActionType, 6>({{
  {ActionType::bet, "bet"}, {ActionType::call, "call"},
  {ActionType::check, "check"}, {ActionType::fold, "fold"},
  {ActionType::raise, "raise"}, {ActionType::none, "none"}
}});

static constexpr auto STREET_MAPPER = makeEnumMapper<Street, 4>({{
  {Street::preflop, "preflop"}, {Street::flop, "flop"},
  {Street::turn, "turn"}, {Street::river, "river"}
}});

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
  return ACTION_TYPE_MAPPER.toString(at);
}

std::string_view toString(Street st) {
  return STREET_MAPPER.toString(st);
}
