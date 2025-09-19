#include "entities/Game.hpp" // Game, std::string
#include "entities/Hand.hpp"
#include "language/assert.hpp" // phudAssert
#include "language/EnumMapper.hpp"

#include <ranges> // std::views

static constexpr auto VARIANT_MAPPER = makeEnumMapper<Variant, 4>({{
  {Variant::holdem, "holdem"}, {Variant::omaha, "omaha"},
  {Variant::omaha5, "omaha5"}, {Variant::none, "none"}
}});

static constexpr auto LIMIT_MAPPER = makeEnumMapper<Limit, 3>({{
  {Limit::noLimit, "no-limit"}, {Limit::potLimit, "pot-limit"},
  {Limit::none, "none"}
}});

std::string_view toString(Variant variant) {
  return VARIANT_MAPPER.toString(variant);
}

std::string_view toString(Limit limitType) {
  return LIMIT_MAPPER.toString(limitType);
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

Game::~Game() = default; // needed because Game owns private std::unique_ptr members

void Game::addHand(std::unique_ptr<Hand> hand) { m_hands.push_back(std::move(hand)); }

std::vector<const Hand*> Game::viewHands() const {
  auto hands_view = m_hands | std::views::transform([](const auto& pHand) { return pHand.get(); });
  return { hands_view.begin(), hands_view.end() };
}

std::vector<const Hand*> Game::viewHands(std::string_view player) const {
  auto r { m_hands
    | std::views::transform([](const auto& pHand) { return pHand.get(); })
    | std::views::filter([player](const auto& pHand) { return pHand->isPlayerInvolved(player); }) };
  return { r.begin(), r.end() };
}

Tournament::Tournament(const Params& p)
  : m_buyIn { p.buyIn },
    m_game { std::make_unique<Game>(Game::Params {.id = p.id, .siteName = p.siteName, .gameName = p.tournamentName, .variant = p.variant, .limitType = p.limit, .isRealMoney = p.isRealMoney, .nbMaxSeats = p.nbMaxSeats, .startDate = p.startDate}) } {
  phudAssert(m_buyIn >= 0, "negative buyIn");
}

Tournament::~Tournament() = default;

void Tournament::addHand(std::unique_ptr<Hand> hand) { m_game->addHand(std::move(hand)); }

CashGame::CashGame(const Params& p)
  : m_smallBlind { p.smallBlind },
    m_bigBlind { p.bigBlind },
    m_game { std::make_unique<Game>(Game::Params{.id = p.id, .siteName = p.siteName, .gameName = p.cashGameName, .variant = p.variant,
                                    .limitType = p.limit, .isRealMoney = p.isRealMoney, .nbMaxSeats = p.nbMaxSeats, .startDate = p.startDate}) } {
  phudAssert(m_smallBlind >= 0, "negative small blind");
  phudAssert(m_bigBlind >= 0, "negative big blind");
}

CashGame::~CashGame() = default;

void CashGame::addHand(std::unique_ptr<Hand> hand) { m_game->addHand(std::move(hand)); }