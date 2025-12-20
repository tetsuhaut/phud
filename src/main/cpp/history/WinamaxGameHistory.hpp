#pragma once

#include <filesystem>
#include <memory> // std::unique_ptr

// forward declarations
class Site;
class PlayerCache;

namespace WinamaxGameHistory {
  // Version with shared cache (for better performance when parsing multiple files)
  [[nodiscard]] std::unique_ptr<Site> parseGameHistory(const std::filesystem::path& gameHistoryFile,
                                                       PlayerCache& cache);
  // Legacy version (creates its own cache)
  [[nodiscard]] std::unique_ptr<Site>
  parseGameHistory(const std::filesystem::path& gameHistoryFile);
  std::unique_ptr<Site> parseGameHistory(auto) = delete;
} // namespace WinamaxGameHistory
