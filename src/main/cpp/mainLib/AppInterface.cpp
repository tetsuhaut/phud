#include "containers/algorithms.hpp"
#include "history/PokerSiteHistory.hpp" // phud::filesystem, std::string_view
#include "mainLib/AppInterface.hpp" // PokerSite
#include "mainLib/ProgramInfos.hpp"

namespace pa = phud::algorithms;
namespace pf = phud::filesystem;

AppInterface::~AppInterface() = default;

/*static*/ bool AppInterface::isPokerApp(std::string_view executableName) {
  const auto& exe { pf::Path(executableName).filename().string() };
  return pa::containsIf(ProgramInfos::POKER_SITE_EXECUTABLE_STEMS,
  [&exe](const auto stem) noexcept { return exe.starts_with(stem); });
}

bool AppInterface::isValidHistory(const pf::Path& historyDir) {
  return PokerSiteHistory::isValidHistory(historyDir);
}