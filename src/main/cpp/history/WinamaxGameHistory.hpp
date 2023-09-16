#pragma once

#include "containers/Tuple.hpp"
#include "filesystem/Filesystem.hpp" // Path
#include "system/memory.hpp" // uptr

class Site;
struct GameData;
enum class Limit : short;
enum class Variant : short;

namespace WinamaxGameHistory {
[[nodiscard]] uptr<Site> parseGameHistory(const Path& gameHistoryFile);
uptr<Site> parseGameHistory(auto) = delete;

}; // namespace WinamaxGameHistory