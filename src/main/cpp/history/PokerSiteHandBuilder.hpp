#pragma once

#include "strings/StringUtils.hpp"

#include <array>

class PlayerCache;
class TextFile;

[[nodiscard]] std::array<std::string, 10> parseSeats(TextFile& tf, PlayerCache& pc);
[[nodiscard]] std::array<std::string_view, 5> split(std::string_view str, std::string_view delimiter);
static constexpr auto SEAT_LENGTH { phud::strings::length("Seat ") };