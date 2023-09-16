#pragma once

#include "strings/StringView.hpp"

enum class [[nodiscard]] GameType : short { none, cashGame, tournament };

[[nodiscard]] StringView toString(GameType gt);
