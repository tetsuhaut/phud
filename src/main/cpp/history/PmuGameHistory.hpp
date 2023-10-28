#pragma once

#include "filesystem/Filesystem.hpp" // Path
#include "system/memory.hpp" // uptr

class Site;
struct GameData;
enum class Limit : short;
enum class Variant : short;

namespace PmuGameHistory {
[[nodiscard]] uptr<Site> parseGameHistory(const phud::filesystem::Path& gameHistoryFile);
uptr<Site> parsePmuGameHistory(auto) = delete;

}; // namespace PmuGameHistory