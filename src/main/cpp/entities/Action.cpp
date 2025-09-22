#include "entities/Action.hpp" // ActionType, std::string
#include "language/EnumMapper.hpp"
#include "language/Validator.hpp"

static constexpr auto ACTION_TYPE_MAPPER = makeEnumMapper<ActionType>(
  std::pair{ActionType::bet, "bet"}, std::pair{ActionType::call, "call"},
  std::pair{ActionType::check, "check"}, std::pair{ActionType::fold, "fold"},
  std::pair{ActionType::raise, "raise"}, std::pair{ActionType::none, "none"}
);

static constexpr auto STREET_MAPPER = makeEnumMapper<Street>(
  std::pair{Street::preflop, "preflop"}, std::pair{Street::flop, "flop"},
  std::pair{Street::turn, "turn"}, std::pair{Street::river, "river"}
);

Action::Action(const Params& p)
  : m_handId { p.handId },
    m_playerName { p.playerName },
    m_street { p.street },
    m_type { p.type },
    m_index { p.actionIndex },
    m_betAmount { p.betAmount } {
  validation::require(Street::none != m_street, "Cannot create 'none' action");
  validation::requireNonEmpty(m_handId, "handId");
  validation::requireNonEmpty(m_playerName, "playerName");
  validation::requirePositive(m_betAmount, "betAmount");
}

Action::~Action() = default;

std::string_view toString(ActionType at) {
  return ACTION_TYPE_MAPPER.toString(at);
}

std::string_view toString(Street st) {
  return STREET_MAPPER.toString(st);
}
