#pragma once

#include "filesystem/Filesystem.hpp" // Path
#include "system/memory.hpp" // uptr

class Site;
struct GameData;
enum class Limit : short;
enum class Variant : short;

namespace WinamaxGameHistory {
[[nodiscard]] uptr<Site> parseGameHistory(const std::filesystem::path& gameHistoryFile);
uptr<Site> parseGameHistory(auto) = delete;

}; // namespace WinamaxGameHistory