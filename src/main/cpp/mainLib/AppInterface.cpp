#include "containers/algorithms.hpp"
#include "history/PokerSiteHistory.hpp" // phud::filesystem, std::string_view
#include "mainLib/AppInterface.hpp" // PokerSite
#include "mainLib/ProgramInfos.hpp"

namespace fs = std::filesystem;
namespace pa = phud::algorithms;

AppInterface::~AppInterface() = default;

/*static*/ bool AppInterface::isPokerApp(std::string_view executableName) {
  const auto& exe { fs::path(executableName).filename().string() };
  return pa::containsIf(ProgramInfos::POKER_SITE_EXECUTABLE_STEMS,
  [&exe](const auto stem) noexcept { return exe.starts_with(stem); });
}

bool AppInterface::isValidHistory(const fs::path& historyDir) {
  return PokerSiteHistory::isValidHistory(historyDir);
}