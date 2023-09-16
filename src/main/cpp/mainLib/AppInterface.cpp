#include "containers/algorithms.hpp"
#include "history/PokerSiteHistory.hpp" // phud::filesystem, StringView
#include "mainLib/AppInterface.hpp" // PokerSite
#include "mainLib/ProgramInfos.hpp"

namespace pa = phud::algorithms;
namespace pf = phud::filesystem;

AppInterface::~AppInterface() = default;

/*static*/ bool AppInterface::isPokerApp(StringView executableName) {
  const auto& exe { Path(executableName).filename().string() };
  return pa::containsIf(ProgramInfos::POKER_SITE_EXECUTABLE_STEMS,
  [&exe](const auto stem) noexcept { return exe.starts_with(stem); });
}

bool AppInterface::isValidHistory(const Path& historyDir) {
  return PokerSiteHistory::isValidHistory(historyDir);
}