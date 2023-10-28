#include "entities/Player.hpp" // Player, std::string_view
#include "language/assert.hpp" // phudAssert

Player::~Player() = default;

Player::Player(const Params& p)
  : m_name { p.name },
    m_site { p.site },
    m_comments {p.comments } {
  phudAssert(!m_name.empty(), "name is empty");
  phudAssert(!m_site.empty(), "site is empty");
}

