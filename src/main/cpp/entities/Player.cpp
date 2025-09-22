#include "entities/Player.hpp" // Player, std::string_view
#include "language/Validator.hpp"

Player::~Player() = default;

Player::Player(const Params& p)
  : m_name { p.name },
    m_site { p.site },
    m_comments {p.comments } {
  validation::requireNonEmpty(m_name, "name");
  validation::requireNonEmpty(m_site, "site");
}
