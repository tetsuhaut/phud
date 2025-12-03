#include "entities/GameType.hpp"
#include "language/EnumMapper.hpp" // std::pair

static constexpr auto GAMETYPE_MAPPER = makeEnumMapper<GameType>(
    std::pair {GameType::cashGame, "cashGame"}, std::pair {GameType::tournament, "tournament"});

std::string_view toString(GameType gt) {
  return GAMETYPE_MAPPER.toString(gt);
}
