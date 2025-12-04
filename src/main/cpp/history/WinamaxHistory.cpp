#include "constants/ProgramInfos.hpp"
#include "entities/Site.hpp"              // Site
#include "filesystem/FileUtils.hpp"       // phud::filesystem::*
#include "history/WinamaxGameHistory.hpp" // parseGameHistory
#include "history/WinamaxHistory.hpp" // WinamaxHistory, std::filesystem::path, fs::*, Global::*, std::string, phud::strings
#include "language/Either.hpp"
#include "log/Logger.hpp"                // CURRENT_FILE_NAME
#include "strings/StringUtils.hpp"       // concatLiteral
#include "threads/PlayerCache.hpp"       // PlayerCache
#include "threads/ThreadPool.hpp"        // Future
#include <stlab/concurrency/utility.hpp> // stlab::await
#include <expected>
#include <ranges>
#include <thread> // std::thread::hardware_concurrency

static Logger& LOG() {
  static Logger logger {CURRENT_FILE_NAME};
  return logger;
}

namespace fs = std::filesystem;
namespace pf = phud::filesystem;
namespace ps = phud::strings;

namespace {
constexpr std::string_view ERR_MSG {"The chosen directory '{}' should contain a {} directory"};

[[nodiscard]] static std::expected<std::vector<fs::path>, std::string>
getHistoryFilesOrErrorMessage(const fs::path& dir, const fs::path& histoDir) {
  if (!pf::isDir(histoDir)) {
    return std::unexpected(fmt::format(ERR_MSG, dir.string(), "'history'"));
  }

  if (!pf::listSubDirs(histoDir).empty()) {
    return std::unexpected(fmt::format(
        "The chosen directory '{}' should contain a {} directory that contains only files",
        dir.string(), "'history'"));
  }

  if (const auto allFilesAndDirs = pf::listFilesAndDirs(histoDir); !allFilesAndDirs.empty()) {
    return allFilesAndDirs;
  }

  return std::unexpected(fmt::format(ERR_MSG, dir.string(), "non empty 'history'"));
}

[[nodiscard]] std::vector<fs::path> getFiles(const fs::path& historyDir) {
  if (WinamaxHistory::isValidHistory(historyDir)) {
    return pf::listTxtFilesInDir(historyDir / "history");
  }

  LOG().error<"The directory '{}' is not a valid Winamax history directory">(historyDir.string());
  return {};
}

[[nodiscard]] std::vector<fs::path> getFiles(auto) = delete; // use only std::filesystem::path

// using auto&& enhances performances by inlining std::function's logic
[[nodiscard]] std::vector<fs::path> getFilesAndNotify(const fs::path& historyDir,
                                                      auto&& onSetNbFiles) {
  const auto files = getFiles(historyDir);

  if (onSetNbFiles) {
    const auto fileSize = files.size();
    LOG().info<"Notify observer of {} files.">(fileSize);

    if (!files.empty()) {
      std::forward<decltype(onSetNbFiles)>(onSetNbFiles)(fileSize);
    }
  }

  return files;
}

// disable other types than const std::filesystem::path&
std::vector<fs::path> getFilesAndNotify(auto, auto) = delete;

std::vector<Future<Site*>> parseFilesAsync(std::span<const fs::path> files, std::atomic_bool& stop,
                                           const auto& onProgress) {
  std::vector<Future<Site*>> ret;
  ret.reserve(files.size());
  std::transform(
      files.cbegin(), files.cend(), std::back_inserter(ret),
      [&onProgress, &stop](const auto& file) {
        if (stop) {
          return Future<Site*>();
        }

        return ThreadPool::submit([file, onProgress, stop = std::ref(stop)]() {
          Site* pSite {nullptr};

          try {
            pSite = WinamaxGameHistory::parseGameHistory(file).release();
          } catch (const std::exception& e) {
            LOG().error<"Exception loading the file {}: {}">(file.filename().string(), e.what());
          } catch (const char* str) {
            LOG().error<"Exception loading the file {}: {}">(file.filename().string(), str);
          }

          if (!stop.get() and onProgress) {
            onProgress();
          }

          return pSite;
        });
      });
  return ret;
}

// Parse files in batches to limit concurrency and reduce memory pressure
// Uses a shared PlayerCache to avoid creating duplicate Player objects
std::vector<Future<Site*>> parseFilesAsyncBatched(std::span<const fs::path> files,
                                                  std::atomic_bool& stop, const auto& onProgress,
                                                  PlayerCache& sharedCache) {
  // Batch size = 2x number of hardware threads to keep all cores busy
  // while limiting memory usage from having too many files loaded at once
  const std::size_t batchSize = std::max(2u, std::thread::hardware_concurrency() * 2);
  std::vector<Future<Site*>> allTasks;
  allTasks.reserve(files.size());
  LOG().debug<"Processing {} files in batches of {}">(files.size(), batchSize);

  // Process files in batches
  for (std::size_t batchStart = 0; batchStart < files.size() and !stop; batchStart += batchSize) {
    const std::size_t batchEnd = std::min(batchStart + batchSize, files.size());
    const std::size_t currentBatchSize = batchEnd - batchStart;
    LOG().debug<"Processing batch {}-{} ({} files)">(batchStart, batchEnd - 1, currentBatchSize);
    // Submit current batch
    std::vector<Future<Site*>> batchTasks;
    batchTasks.reserve(currentBatchSize);

    for (std::size_t i = batchStart; i < batchEnd and !stop; ++i) {
      const auto file = files[i];
      batchTasks.push_back(
          ThreadPool::submit([file, onProgress, stop = std::ref(stop), &sharedCache]() {
            Site* pSite = nullptr;

            try {
              // Use shared cache to avoid creating duplicate players
              pSite = WinamaxGameHistory::parseGameHistory(file, sharedCache).release();
            } catch (const std::exception& e) {
              LOG().error<"Exception loading the file {}: {}">(file.filename().string(), e.what());
            } catch (const char* str) {
              LOG().error<"Exception loading the file {}: {}">(file.filename().string(), str);
            }

            if (!stop.get() and onProgress) {
              onProgress();
            }

            return pSite;
          }));
    }

    // Wait for current batch to complete before starting next batch
    std::ranges::for_each(batchTasks, [](auto& task) {
      if (task.valid()) {
        stlab::await(stlab::copy(task)); // Wait for task to complete
      }
    });

    // Move completed tasks to result vector
    std::ranges::move(batchTasks, std::back_inserter(allTasks));
  }

  return allTasks;
}
} // anonymous namespace

struct [[nodiscard]] WinamaxHistory::Implementation final {
  std::vector<Future<Site*>> m_tasks = {};
  std::atomic_bool m_stop = true;
}; // struct WinamaxHistory::Implementation

WinamaxHistory::WinamaxHistory() noexcept
  : m_pImpl {std::make_unique<Implementation>()} {}

WinamaxHistory::~WinamaxHistory() {
  try {
    stopLoading();
  } catch (...) { // can't throw in a destructor
    LOG().error<"Unknown Error raised by WinamaxHistory::stopLoading().">();
  }
}

/**
 * @return true if the given dir is an existing dir, contains a 'history' subdir which only contains
 * txt files, and if it contains a 'winamax_positioning_file.dat' file.
 */
/*static*/
bool WinamaxHistory::isValidHistory(const fs::path& dir) {
  LOG().debug<__func__>();
  const auto histoDir = (dir / "history").lexically_normal();
  const auto& either = getHistoryFilesOrErrorMessage(dir, histoDir);

  if (!either) {
    LOG().error(either.error());
    return false; // do not throw as functionnaly correct
  }

  const auto allFilesAndDirs = either.value();
  return pf::containsAFileEndingWith(allFilesAndDirs, "winamax_positioning_file.dat");
}

std::unique_ptr<Site> WinamaxHistory::load(const fs::path& dir, std::function<void()> onProgress,
                                           std::function<void(std::size_t)> onSetNbFiles) {
  m_pImpl->m_stop = false;

  try {
    LOG().debug<"Loading the history dir '{}'.">(dir.string());
    const auto files = getFilesAndNotify(dir, onSetNbFiles);
    auto ret = std::make_unique<Site>(ProgramInfos::WINAMAX_SITE_NAME);

    if (files.empty()) {
      LOG().error<"0 file found in the dir {}">(dir.string());
      return ret;
    }

    LOG().info<"{} file{} to load.">(files.size(), ps::plural(files.size()));

    // Create a shared PlayerCache to avoid creating duplicate Player objects
    PlayerCache sharedCache {ProgramInfos::WINAMAX_SITE_NAME};

    // Use batched parsing to limit concurrency and memory usage
    m_pImpl->m_tasks = parseFilesAsyncBatched(files, m_pImpl->m_stop, onProgress, sharedCache);
    LOG().info<"Merging results from {} tasks.">(m_pImpl->m_tasks.size());

    // Merge all game data from parsed files
    std::ranges::for_each(m_pImpl->m_tasks, [&ret, this](auto& task) {
      if (task.valid()) {
        if (const auto site = std::unique_ptr<Site>(stlab::await(std::move(task)));
            !m_pImpl->m_stop and site) {
          ret->merge(*site);
        }
      }
    });
    m_pImpl->m_tasks.clear();
    // Extract all players from shared cache and add to result
    auto players = sharedCache.extractPlayers();
    LOG().info<"Adding {} player{} from shared cache.">(players.size(), ps::plural(players.size()));
    std::ranges::for_each(players, [&](auto& p) { ret->addPlayer(std::move(p)); });
    LOG().info<"Loading done.">();
    return ret;
  } catch (const std::exception& e) {
    LOG().error<"Exception in WinamaxHistory::load: {}">(e.what());
    return std::make_unique<Site>(ProgramInfos::WINAMAX_SITE_NAME);
  }
}

/* [[nodicard]] static */
std::unique_ptr<Site> WinamaxHistory::load(const fs::path& dir) {
  WinamaxHistory wh;
  return wh.load(dir, nullptr, nullptr);
}

void WinamaxHistory::stopLoading() {
  m_pImpl->m_stop = true;
  std::size_t nbTasksFinished = 0;

  while (nbTasksFinished != m_pImpl->m_tasks.size()) {
    std::ranges::for_each(m_pImpl->m_tasks, [&nbTasksFinished](auto& task) {
      if (task.is_ready()) {
        task.reset(); // it won't be ready anymore
        nbTasksFinished++;
      }
    });
  }
}

std::unique_ptr<Site> WinamaxHistory::reloadFile(const fs::path& file) {
  LOG().trace<"Reloading the history file '{}'.">(file.string());
  std::unique_ptr<Site> ret = nullptr;

  try {
    ret = WinamaxGameHistory::parseGameHistory(file);
  } catch (const std::exception& e) {
    LOG().error<"Exception loading the file {}: {}">(file.string(), e.what());
  }

  return ret;
}

std::string_view
WinamaxHistory::getTableNameFromTableWindowTitle(std::string_view tableWindowTitle) const {
  // Remove "Winamax " prefix if present (new format in 2025)
  auto workingTitle = tableWindowTitle;
  if (constexpr auto winamaxPrefix = std::string_view("Winamax ");
      workingTitle.starts_with(winamaxPrefix)) {
    workingTitle = workingTitle.substr(winamaxPrefix.size());
  }

  // Extract table name using existing logic
  const auto pos = workingTitle.find('#');
  return (notFound(pos)) ? workingTitle.substr(0, workingTitle.find(" / "))
                         : workingTitle.substr(0, pos);
}

std::optional<fs::path>
WinamaxHistory::getHistoryFileFromTableWindowTitle(const fs::path& dir,
                                                   std::string_view tableWindowTitle) const {
  const auto tableName = getTableNameFromTableWindowTitle(tableWindowTitle);
  // Search for all files matching the table name, regardless of game type
  const auto tablePattern = fmt::format("*_{}_*.txt", tableName);
  auto files = pf::listFilesMatchingPattern(dir / "history", tablePattern);

  if (files.empty()) {
    LOG().error<"No history file found for table '{}'">(tableName);
    return {};
  } else {
    // Take the most recent file (sort by modification time, most recent last)
    std::ranges::sort(files, pf::PathModificationTimeComparator {});
  }
  const auto candidate = files.back();
  LOG().info<"Found {} history files for table '{}', using most recent: {}">(
      files.size(), tableName, candidate.string());
  return std::optional<fs::path> {candidate};
}
