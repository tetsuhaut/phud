#pragma once

#include "strings/StringUtils.hpp"

enum class [[nodiscard]] GameType : short { none, cashGame, tournament };

[[nodiscard]] std::string_view toString(GameType gt);
