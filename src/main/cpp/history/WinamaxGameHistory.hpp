#pragma once

#include <filesystem>
#include <memory> // std::unique_ptr

// forward declaration
class Site;
struct GameData;
enum class Limit : short;
enum class Variant : short;

namespace WinamaxGameHistory {
[[nodiscard]] std::unique_ptr<Site> parseGameHistory(const std::filesystem::path& gameHistoryFile);
std::unique_ptr<Site> parseGameHistory(auto) = delete;

}; // namespace WinamaxGameHistory