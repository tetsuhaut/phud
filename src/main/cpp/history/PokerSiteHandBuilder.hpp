#pragma once

#include "mainLib/ProgramInfos.hpp"
#include "strings/String.hpp"
#include "threads/PlayerCache.hpp"
#include <array>

class PlayerCache;
class TextFile;

[[nodiscard]] std::array<String, 10> parseSeats(TextFile& tf, PlayerCache& pc);
[[nodiscard]] std::array<StringView, 5> split(StringView str, StringView delimiter);
static constexpr auto SEAT_LENGTH { phud::strings::length("Seat ") };