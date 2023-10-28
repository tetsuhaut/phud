#include "containers/algorithms.hpp"
#include "entities/Game.hpp" // Game, String, mkView
#include "entities/Hand.hpp"
#include "language/assert.hpp" // phudAssert
#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;

// Note : must use frozen::string when it is a map key.
// frozen::string can be created from std::string_view.

static constexpr auto VARIANT_TO_STRING {
  frozen::make_unordered_map<Variant, std::string_view>({
    { Variant::holdem, "holdem" }, { Variant::omaha, "omaha" },
    { Variant::omaha5, "omaha5" }, { Variant::none, "none" }
  })
};

static constexpr auto LIMIT_TO_STRING {
  frozen::make_unordered_map<Limit, std::string_view>({
    { Limit::noLimit, "no-limit" }, { Limit::potLimit, "pot-limit" },
    { Limit::none, "none" }
  })
};

std::string_view toString(Variant variant) {
  return pa::getValueFromKey(VARIANT_TO_STRING, variant);
}

std::string_view toString(Limit limitType) {
  return pa::getValueFromKey(LIMIT_TO_STRING, limitType);
}

Game::Game(const Params& args)
  : m_id { args.id },
    m_site { args.siteName },
    m_name { args.gameName },
    m_variant { args.variant },
    m_limitType { args.limitType },
    m_isRealMoney { args.isRealMoney },
    m_nbMaxSeats { args.nbMaxSeats },
    m_startDate { args.startDate },
    m_hands {} {
  phudAssert(!m_id.empty(), "empty id");
  phudAssert(!m_site.empty(), "empty site");
  phudAssert(!m_name.empty(), "empty name");
  phudAssert(m_nbMaxSeats != Seat::seatUnknown, "nbMaxSeats must be defined");
}

Game::~Game() = default; // needed because Game owns private uptr members

void Game::addHand(uptr<Hand> hand) { m_hands.push_back(std::move(hand)); }

std::vector<const Hand*> Game::viewHands() const { return pa::mkView(m_hands); }

std::vector<const Hand*> Game::viewHands(std::string_view player) const {
  std::vector<const Hand*> ret;
  pa::forEach(m_hands, [&](const auto & h) { if (h->isPlayerInvolved(player)) { ret.push_back(h.get()); } });
  return ret;
}

Tournament::Tournament(const Params& p)
  : m_buyIn { p.buyIn },
    m_game { mkUptr<Game>(Game::Params {.id = p.id, .siteName = p.siteName, .gameName = p.tournamentName, .variant = p.variant, .limitType = p.limit, .isRealMoney = p.isRealMoney, .nbMaxSeats = p.nbMaxSeats, .startDate = p.startDate}) } {
  phudAssert(m_buyIn >= 0, "negative buyIn");
}

Tournament::~Tournament() = default;

void Tournament::addHand(uptr<Hand> hand) { m_game->addHand(std::move(hand)); }

CashGame::CashGame(const Params& p)
  : m_smallBlind { p.smallBlind },
    m_bigBlind { p.bigBlind },
    m_game { mkUptr<Game>(Game::Params{.id = p.id, .siteName = p.siteName, .gameName = p.cashGameName, .variant = p.variant,
                                       .limitType = p.limit, .isRealMoney = p.isRealMoney, .nbMaxSeats = p.nbMaxSeats, .startDate = p.startDate}) } {
  phudAssert(m_smallBlind >= 0, "negative small blind");
  phudAssert(m_bigBlind >= 0, "negative big blind");
}

CashGame::~CashGame() = default;

void CashGame::addHand(uptr<Hand> hand) { m_game->addHand(std::move(hand)); }