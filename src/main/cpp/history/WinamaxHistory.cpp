#include "containers/algorithms.hpp" // phud::algorithms::*
#include "db/Database.hpp"          // Database
#include "entities/Action.hpp"      // Action, ActionType
#include "entities/Card.hpp"        // Card, toCard
#include "entities/Game.hpp"        // Game
#include "entities/Player.hpp"      // Player
#include "entities/Site.hpp"        // Site
#include "history/WinamaxGameHistory.hpp" // parseGameHistory
#include "history/WinamaxHistory.hpp" // WinamaxHistory, Path, fs::*, Global::*, String, phud::strings
#include "language/Either.hpp"
#include "language/limits.hpp" // toInt
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/ProgramInfos.hpp"
#include "strings/StringUtils.hpp" // concatLiteral
#include "threads/ThreadPool.hpp" // Future
#include <stlab/concurrency/utility.hpp> // stlab::await

static Logger LOG { CURRENT_FILE_NAME };

namespace pa = phud::algorithms;
namespace pf = phud::filesystem;
namespace ps = phud::strings;

struct [[nodiscard]] WinamaxHistory::Implementation final {
  std::vector<Future<Site*>> m_tasks {};
  std::atomic_bool m_stop { true };
}; // struct WinamaxHistory::Implementation

WinamaxHistory::WinamaxHistory() noexcept : m_pImpl { mkUptr<Implementation>() }  {}

WinamaxHistory::~WinamaxHistory() {
  try {
    stopLoading();
  } catch (...) {
    std::exit(4);
  }
}

static constexpr std::string_view ERR_MSG { "The chosen directory '{}' should contain a {} directory" };

[[nodiscard]] static Either<std::string, std::vector<Path>> getErrorMessageOrHistoryFiles(
const Path& dir, const Path& histoDir) {
  if (!pf::isDir(histoDir)) {
    return Either<std::string, std::vector<Path>>::left(fmt::format(ERR_MSG, dir.string(), "'history'"));
  }

  if (!pf::isDir(dir / "data" / "buddy")) {
    return Either<std::string, std::vector<Path>>::left(fmt::format(ERR_MSG,  dir.string(), "'data/buddy'"));
  }

  if (!pf::isDir(dir / "data" / "players")) {
    return Either<std::string, std::vector<Path>>::left(fmt::format(ERR_MSG, dir.string(), "'data/players'"));
  }

  if (!pf::listSubDirs(histoDir).empty()) {
    return Either<std::string, std::vector<Path>>::left(
             fmt::format("The chosen directory '{}' should contain a {} directory that contains only files", dir.string(), "'history'"));
  }

  if (const auto & allFilesAndDirs { pf::listFilesAndDirs(histoDir) }; !allFilesAndDirs.empty()) {
    return Either<std::string, std::vector<Path>>::right(allFilesAndDirs);
  }

  return Either<std::string, std::vector<Path>>::left(
           fmt::format(ERR_MSG, dir.string(), "non empty 'history'"));
}

/**
 * @return true if the given dir is an existing dir, contains a 'history' subdir which only contains txt files,
 * and if it contains a 'winamax_positioning_file.dat' file.
 */
/*static*/ bool WinamaxHistory::isValidHistory(const Path& dir) {
  const auto& histoDir { (dir / "history").lexically_normal() };
  const auto& either { getErrorMessageOrHistoryFiles(dir, histoDir) };

  if (either.isLeft()) {
    LOG.error(either.getLeft());
    return false; // do not throw as functionnaly correct
  }

  const auto& allFilesAndDirs { either.getRight() };
  return pf::containsAFileEndingWith(allFilesAndDirs, "winamax_positioning_file.dat");
}

[[nodiscard]] static inline std::vector<Path> getFiles(const Path& historyDir) {
  if (WinamaxHistory::isValidHistory(historyDir)) { return pf::listTxtFilesInDir(historyDir / "history"); }

  LOG.error<"The directory '{}' is not a valid Winamax history directory">(historyDir.string());
  return {};
}
[[nodiscard]] static inline std::vector<Path> getFiles(auto) = delete; // use only Path

// using auto&& enhances performances by inlining std::function's logic
[[nodiscard]] static inline std::vector<Path> getFilesAndNotify(const Path& historyDir,
    auto&& setNbFilesCb) {
  const auto& files { getFiles(historyDir) };

  if (setNbFilesCb) {
    const auto fileSize{ files.size() };
    LOG.info<"Notify observer of {} files.">(fileSize);

    if (!files.empty()) { std::forward<decltype(setNbFilesCb)>(setNbFilesCb)(fileSize); }
  }

  return files;
}

// disable other types than const Path&
static inline std::vector<Path> getFilesAndNotify(auto, auto) = delete;

static inline std::vector<Future<Site*>> parseFilesAsync(std::span<const Path> files,
std::atomic_bool& stop, const auto& incrementCb) {
  std::vector<Future<Site*>> ret;
  ret.reserve(files.size());
  pa::transform(files, ret, [&incrementCb, &stop](const auto & file) {
    if (stop) { return Future<Site*>(); }

    return ThreadPool::submit([&file, &incrementCb, &stop]() {
      Site* pSite { nullptr };

      try {
        pSite = WinamaxGameHistory::parseGameHistory(file).release();
      } catch (const std::exception& e) {
        LOG.error<"Exception loading the file {}: {}">(file.filename().string(), e.what());
      } catch (const char* str) {
        LOG.error<"Exception loading the file {}: {}">(file.filename().string(), str);
      }

      if (!stop and incrementCb) { incrementCb(); }

      return pSite;
    });
  });
  return ret;
}

uptr<Site> WinamaxHistory::load(const Path& winamaxHistoryDir,
                                FunctionVoid incrementCb,
                                FunctionInt setNbFilesCb) {
  m_pImpl->m_stop = false;

  try {
    LOG.debug<"Loading the history dir '{}'.">(winamaxHistoryDir.string());
    const auto& files { getFilesAndNotify(winamaxHistoryDir, setNbFilesCb) };
    auto ret { mkUptr<Site>(ProgramInfos::WINAMAX_SITE_NAME) };

    if (files.empty()) {
      LOG.error<"0 file found in the dir {}">(winamaxHistoryDir.string());
      return ret;
    }

    LOG.info<" file{} to load.">(files.size(), ps::plural(files.size()));
    m_pImpl->m_tasks = parseFilesAsync(files, m_pImpl->m_stop, incrementCb);
    LOG.info<"Waiting for the end of loading.">();
    pa::forEach(m_pImpl->m_tasks, [&ret, this](auto & task) {
      if (task.valid()) {
        uptr<Site> s { stlab::await(std::move(task)) };

        if (!m_pImpl->m_stop and s) { ret->merge(*s); }
      }
    });
    m_pImpl->m_tasks.clear();
    LOG.info<"Loading done.">();
    return ret;
  } catch (const std::exception& e) {
    LOG.error<"Exception in WinamaxHistory::load: {}">(e.what());
    return mkUptr<Site>(ProgramInfos::WINAMAX_SITE_NAME);
  }
}

/* [[nodicard]] static */ uptr<Site> WinamaxHistory::load(const Path& historyDir) {
  WinamaxHistory wh;
  return wh.load(historyDir, nullptr, nullptr);
}

void WinamaxHistory::stopLoading() {
  m_pImpl->m_stop = true;
  std::size_t nbTasksFinished { 0 };

  while (nbTasksFinished != m_pImpl->m_tasks.size()) {
    pa::forEach(m_pImpl->m_tasks, [&nbTasksFinished](auto & task) {
      if (task.is_ready()) {
        task.reset(); // it won't be ready anymore
        nbTasksFinished++;
      }
    });
  }
}

uptr<Site> WinamaxHistory::reloadFile(const Path& file) {
  LOG.trace<"Reloading the history file '{}'.">(file.string());
  uptr<Site> ret { nullptr };

  try {
    ret = WinamaxGameHistory::parseGameHistory(file);
  } catch (const std::exception& e) {
    LOG.error<"Exception loading the file {}: {}">(file.string(), e.what());
  }

  return ret;
}

std::string_view WinamaxHistory::getTableNameFromTableWindowTitle(std::string_view tableWindowTitle) const {
  const auto pos { tableWindowTitle.find("#") };
  return (notFound(pos)) ? tableWindowTitle.substr(0,
         tableWindowTitle.find(" / ")) : tableWindowTitle.substr(0, pos);
}

static constexpr bool isReal(std::string_view tableWindowTitle) noexcept { return !tableWindowTitle.ends_with("fictif"); }

static inline std::string getGameType(std::string_view tableWindowTitle) {
  if (ps::contains(tableWindowTitle, "NL Holdem")) { return "holdem_no-limit"; }
  // TODO
  else {
    LOG.error<"Unknown game type for table window title {}">(tableWindowTitle);
    return "omaha5_pot-limit";
  }
}

Path WinamaxHistory::getHistoryFileFromTableWindowTitle(const Path& historyDir,
    std::string_view tableWindowTitle) const {
  const auto tableName { getTableNameFromTableWindowTitle(tableWindowTitle) };
  const auto& reality { isReal(tableWindowTitle) ? "real" : "play" };
  const auto& game { getGameType(tableWindowTitle) };
  const auto& postfix { fmt::format("_{}_{}_{}.txt", tableName, reality, game)};
  auto files { pf::listFilesInDir(historyDir / "history", postfix)};

  if (files.empty()) {
    LOG.error<"No history file found for table\n'{}'\nlooking in directory {}\nhaving the postfix '{}'.">
    (
      tableWindowTitle, (historyDir / "history").string(), postfix);
    return "";
  }

  if (1 == files.size()) { return files.front(); }

  std::sort(std::begin(files), std::end(files), pf::PathComparator {});
  return files.back();
}