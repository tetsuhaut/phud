#pragma once

#include <string_view>

enum class [[nodiscard]] GameType : short { none, cashGame, tournament };

[[nodiscard]] std::string_view toString(GameType gt);
