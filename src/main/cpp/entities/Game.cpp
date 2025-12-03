#include "entities/Game.hpp" // Game, std::string
#include "entities/Hand.hpp"
#include "entities/Seat.hpp"
#include "language/EnumMapper.hpp"
#include "language/Validator.hpp"

#include <ranges> // std::views

static constexpr auto VARIANT_MAPPER = makeEnumMapper<Variant>(
    std::pair {Variant::holdem, "holdem"}, std::pair {Variant::omaha, "omaha"},
    std::pair {Variant::omaha5, "omaha5"}, std::pair {Variant::none, "none"});

static constexpr auto LIMIT_MAPPER = makeEnumMapper<Limit>(std::pair {Limit::noLimit, "no-limit"},
                                                           std::pair {Limit::potLimit, "pot-limit"},
                                                           std::pair {Limit::none, "none"});

std::string_view toString(Variant variant) {
  return VARIANT_MAPPER.toString(variant);
}

std::string_view toString(Limit limitype) {
  return LIMIT_MAPPER.toString(limitype);
}

Game::Game(const Params& args)
  : m_id {args.id},
    m_site {args.siteName},
    m_name {args.gameName},
    // ReSharper disable once CppRedundantMemberInitializer
    m_hands {},
    m_startDate {args.startDate},
    m_variant {args.variant},
    m_limitType {args.limitType},
    m_nbMaxSeats {args.nbMaxSeats},
    m_isRealMoney {args.isRealMoney} {
  validation::requireNonEmpty(m_id, "id");
  validation::requireNonEmpty(m_site, "site");
  validation::requireNonEmpty(m_name, "name");
  validation::require(m_nbMaxSeats != Seat::seatUnknown, "nbMaxSeats must be defined");
}

Game::~Game() = default; // needed because Game owns private std::unique_ptr members

void Game::addHand(std::unique_ptr<Hand> hand) {
  m_hands.push_back(std::move(hand));
}

std::vector<const Hand*> Game::viewHands() const {
  const auto handsView {m_hands |
                        std::views::transform([](const auto& pHand) { return pHand.get(); })};
  return {handsView.begin(), handsView.end()};
}

std::vector<const Hand*> Game::viewHands(std::string_view player) const {
  auto r {
      m_hands | std::views::transform([](const auto& pHand) { return pHand.get(); }) |
      std::views::filter([player](const auto& pHand) { return pHand->isPlayerInvolved(player); })};
  return {r.begin(), r.end()};
}

Tournament::Tournament(const Params& p)
  : m_game {std::make_unique<Game>(Game::Params {.id = p.id,
                                                 .siteName = p.siteName,
                                                 .gameName = p.tournamentName,
                                                 .variant = p.variant,
                                                 .limitType = p.limit,
                                                 .isRealMoney = p.isRealMoney,
                                                 .nbMaxSeats = p.nbMaxSeats,
                                                 .startDate = p.startDate})},
    m_buyIn {p.buyIn} {
  validation::requirePositive(m_buyIn, "buyIn");
}

Tournament::~Tournament() = default;

void Tournament::addHand(std::unique_ptr<Hand> hand) const {
  m_game->addHand(std::move(hand));
}

CashGame::CashGame(const Params& p)
  : m_game {std::make_unique<Game>(Game::Params {.id = p.id,
                                                 .siteName = p.siteName,
                                                 .gameName = p.cashGameName,
                                                 .variant = p.variant,
                                                 .limitType = p.limit,
                                                 .isRealMoney = p.isRealMoney,
                                                 .nbMaxSeats = p.nbMaxSeats,
                                                 .startDate = p.startDate})},
    m_smallBlind {p.smallBlind},
    m_bigBlind {p.bigBlind} {
  validation::requirePositive(m_smallBlind, "smallBlind");
  validation::requirePositive(m_bigBlind, "bigBlind");
}

CashGame::~CashGame() = default;

void CashGame::addHand(std::unique_ptr<Hand> hand) const {
  m_game->addHand(std::move(hand));
}
