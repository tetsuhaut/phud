#include "entities/Action.hpp"
#include "entities/Hand.hpp"
#include "language/Validator.hpp"
#include "strings/StringUtils.hpp" // phud::strings::*
#include <algorithm>               // std::ranges::find_if, std::ranges::any_of
#include <ranges>                  // std::views
#include <vector>

namespace ps = phud::strings;

Hand::Hand(Params& p)
  : m_seats {p.seatPlayers},
    m_id {p.id},
    m_siteName {p.siteName},
    m_tableName {p.tableName},
    m_actions {std::move(p.actions)},
    m_date {p.startDate},
    m_ante {p.ante},
    m_level {p.level},
    m_heroCards {p.heroCards},
    m_boardCards {p.boardCards},
    m_gameType {p.gameType},
    m_buttonSeat {p.buttonSeat},
    m_maxSeats {p.maxSeats} {
  validation::requireNonEmpty(m_id, "id");
  validation::requireNonEmpty(m_siteName, "site");
  validation::requireNonEmpty(m_tableName, "table");
  validation::requirePositive(m_ante, "ante");

  std::size_t nbWinners = 0;

  for (const auto& pAction : m_actions) {
    if (ActionType::win == pAction->getType()) {
      const auto& winnerName = pAction->getPlayerName();
      const auto knownWinners = m_winners | std::views::take(nbWinners);
      const auto isAlreadyKnown = std::ranges::any_of(
          knownWinners, [&winnerName](const auto& known) { return winnerName == known; });

      if (!isAlreadyKnown) {
        m_winners.at(nbWinners) = winnerName;
        nbWinners++;
      }
    }
  }
}

Hand::~Hand() = default; // needed because Hand owns a private std::shared_ptr member

bool Hand::isPlayerInvolved(std::string_view name) const {
  const auto isPlayerName {[&name](const auto& a) { return name == a->getPlayerName(); }};
  return std::ranges::find_if(m_actions, isPlayerName) != m_actions.end();
}

bool Hand::isWinner(std::string_view playerName) const noexcept {
  return std::ranges::find(m_winners, playerName.data()) != m_winners.end();
}

double Hand::getWinAmount(std::string_view playerName) const noexcept {
  const auto isWinningActionForPlayer = [playerName](const auto& pAction) {
    return ActionType::win == pAction->getType() and playerName == pAction->getPlayerName();
  };
  const auto it = std::ranges::find_if(m_actions, isWinningActionForPlayer);
  return m_actions.end() == it ? 0.0 : (*it)->getAmount();
}

[[nodiscard]] std::vector<const Action*> Hand::viewActions() const {
  auto actions_view =
      m_actions | std::views::transform([](const auto& pAction) { return pAction.get(); });
  return {actions_view.begin(), actions_view.end()};
}
