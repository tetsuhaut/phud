#include "entities/Action.hpp"
#include "entities/Card.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp"
#include "language/assert.hpp" // phudAssert
#include "strings/StringUtils.hpp" // phud::strings::*

#include <algorithm> // std::transform
#include <iterator> // std:: back_insertor
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
  phudAssert(!m_id.empty(), "id is empty");
  phudAssert(!m_siteName.empty(), "site is empty");
  phudAssert(!m_tableName.empty(), "table is empty");
  phudAssert(m_ante >= 0, "ante is negative");
}

Hand::~Hand() = default; // needed because Hand owns a private std::shared_ptr member

bool Hand::isPlayerInvolved(std::string_view name) const {
  const auto isPlayerName { [&name](const auto & a) { return name == a->getPlayerName(); } };
  return std::end(m_actions) != std::find_if(std::begin(m_actions), std::end(m_actions),
         isPlayerName);
}

bool Hand::isWinner(std::string_view playerName) const noexcept {
  return std::end(m_winners) != std::find(std::begin(m_winners), std::end(m_winners),
                                          playerName.data());
}

[[nodiscard]] std::vector<const Action*> Hand::viewActions() const {
  std::vector<const Action*> ret;
  ret.reserve(m_actions.size());
  std::transform(m_actions.cbegin(), m_actions.end(),
  std::back_inserter(ret), [](const auto & pAction) { return pAction.get(); });
  return ret;
}