#pragma once

#include <string_view>

// fastFold: cash game where a player changes table after each fold (Winamax Escape, ex Go Fast)
enum class [[nodiscard]] GameType : short { none, cashGame, tournament, fastFold };

[[nodiscard]] std::string_view toString(GameType gt);
