#include "entities/GameType.hpp"
#include "language/EnumMapper.hpp"

static constexpr auto GAMETYPE_MAPPER = makeEnumMapper<GameType, 2>({{
  {GameType::cashGame, "cashGame"}, {GameType::tournament, "tournament"}
}});

std::string_view toString(GameType gt) {
  return GAMETYPE_MAPPER.toString(gt);
}
