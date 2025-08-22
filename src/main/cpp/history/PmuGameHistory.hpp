#pragma once

#include <filesystem> // std::filesystem::path
#include <memory> // std::unique_ptr

class Site;
struct GameData;
enum class Limit : short;
enum class Variant : short;

namespace PmuGameHistory {
[[nodiscard]] std::unique_ptr<Site> parseGameHistory(const std::filesystem::path& gameHistoryFile);
std::unique_ptr<Site> parsePmuGameHistory(auto) = delete;

} // namespace PmuGameHistory