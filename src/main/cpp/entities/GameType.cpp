#include "containers/algorithms.hpp"
#include "entities/GameType.hpp"
#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;

static constexpr auto ENUM_TO_STRING {
  frozen::make_unordered_map<GameType, std::string_view>({
    { GameType::cashGame, "cashGame" }, { GameType::tournament, "tournament" }
  })
};

std::string_view toString(GameType gt) {
  return pa::getValueFromKey(ENUM_TO_STRING, gt);
}
