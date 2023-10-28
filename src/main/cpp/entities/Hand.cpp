#include "containers/algorithms.hpp" // phud::contains, mkView
#include "entities/Action.hpp"
#include "entities/Card.hpp"
#include "entities/GameType.hpp"
#include "entities/Hand.hpp"
#include "language/assert.hpp" // phudAssert

namespace pa = phud::algorithms;
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

Hand::~Hand() = default; // needed because Hand owns a private sptr member

bool Hand::isPlayerInvolved(std::string_view name) const {
  return pa::containsIf(m_actions,
  [&name](const auto & a) { return name == a->getPlayerName(); });
}

bool Hand::isWinner(std::string_view playerName) const noexcept {
  return pa::contains(m_winners, playerName.data());
}

[[nodiscard]] std::vector<const Action*> Hand::viewActions() const { return pa::mkView(m_actions); }