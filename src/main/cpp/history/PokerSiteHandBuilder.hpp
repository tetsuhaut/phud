#pragma once

#include "constants/TableConstants.hpp"
#include <array>
#include <string_view>

class PlayerCache;
class TextFile;

[[nodiscard]] std::array<std::string, TableConstants::MAX_SEATS> parseSeats(TextFile& tf, PlayerCache& pc);
[[nodiscard]] std::array<std::string_view, TableConstants::MAX_CARDS> split(std::string_view str,
    std::string_view delimiter);