#pragma once

#include <filesystem>
#include <memory> // std::unique_ptr

// forward declaration
class Site;

namespace WinamaxGameHistory {
[[nodiscard]] std::unique_ptr<Site> parseGameHistory(const std::filesystem::path& gameHistoryFile);
std::unique_ptr<Site> parseGameHistory(auto) = delete;

} // namespace WinamaxGameHistory