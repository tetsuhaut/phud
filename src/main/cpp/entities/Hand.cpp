#include "entities/Action.hpp"
#include "entities/Card.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp"
#include "language/FieldValidators.hpp"
#include "strings/StringUtils.hpp" // phud::strings::*

#include <algorithm> // std::ranges::find_if
#include <ranges> // std::views
#include <vector>

namespace ps = phud::strings;

Hand::Hand(Params& p)
  : m_id { p.id },
    m_gameType { p.gameType },
    m_siteName { p.siteName },
    m_tableName { p.tableName },
    m_buttonSeat { p.buttonSeat },
    m_maxSeats { p.maxSeats },
    m_level { p.level },
    m_ante { p.ante },
    m_date { p.startDate },
    m_heroCards { p.heroCards },
    m_boardCards { p.boardCards },
    m_seats { p.seatPlayers },
    m_actions { std::move(p.actions) },
    m_winners { p.winners } {
  validation::requireNonEmpty(m_id, "id");
  validation::requireNonEmpty(m_siteName, "site");
  validation::requireNonEmpty(m_tableName, "table");
  validation::requirePositive(m_ante, "ante");
}

Hand::~Hand() = default; // needed because Hand owns a private std::shared_ptr member

bool Hand::isPlayerInvolved(std::string_view name) const {
  const auto isPlayerName { [&name](const auto & a) { return name == a->getPlayerName(); } };
  return std::ranges::find_if(m_actions, isPlayerName) != m_actions.end();
}

bool Hand::isWinner(std::string_view playerName) const noexcept {
  return std::ranges::find(m_winners, playerName.data()) != m_winners.end();
}

[[nodiscard]] std::vector<const Action*> Hand::viewActions() const {
  auto actions_view = m_actions | std::views::transform([](const auto& pAction) { return pAction.get(); });
  return { actions_view.begin(), actions_view.end() };
}