#include "constants/ProgramInfos.hpp"
#include "db/Database.hpp"
#include "gui/Gui.hpp"
#include "gui/HistoryService.hpp"
#include "gui/TableService.hpp"
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"
#include "threads/ThreadSafeQueue.hpp" // std::unique_ptr

static Logger LOG { CURRENT_FILE_NAME };

namespace fs = std::filesystem;

namespace {
struct [[nodiscard]] LoggingConfig final {
  LoggingConfig() { Logger::setupConsoleDebugLogging("[%D %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v"); }
  ~LoggingConfig() { Logger::shutdownLogging(); }
};
} // anonymous namespace

class [[nodiscard]] NoOpTableService final : public TableService {
private:
  ThreadSafeQueue<TableStatistics> m_stats {};
  PeriodicTaskStatus m_continue { PeriodicTaskStatus::stopTask };
  PeriodicTask m_task { std::chrono::milliseconds(500), CURRENT_FILE_NAME };

public:
  // TableService interface
  bool isPokerApp(std::string_view /*executableName*/) const override { return true; }

  std::string startProducingStats(std::string_view /*table*/,
                                  std::function < void(TableStatistics&& ts) > /*observer*/) override {
    LOG.debug<__func__>();
    m_continue = PeriodicTaskStatus::repeatTask;
    m_task.start([this]() {
      LOG.debug<"task in guiDryRun startProducingStats()">();
      std::array<std::unique_ptr<PlayerStatistics>, TableConstants::MAX_SEATS> fakeStats{
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player0", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = true,  .nbHands = 10, .vpip = 1, .pfr = 7 }),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player1", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 20, .vpip = 2, .pfr = 8 }),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player2", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 30, .vpip = 3, .pfr = 9 }),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player3", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 40, .vpip = 4, .pfr = 10}),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player4", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 50, .vpip = 5, .pfr = 11}),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player5", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 60, .vpip = 6, .pfr = 12}),
        nullptr, nullptr, nullptr, nullptr
      };
      m_stats.push(TableStatistics { Seat::seatSix, std::move(fakeStats) });
      LOG.debug<"the task in guiDryRun startObservingTable() returns {}">(toString(m_continue));
      return m_continue; // run until the NoOpApp object is destroyed
    });
    return "";
  }

  void stopProducingStats() override {
    LOG.debug<__func__>();
    m_continue = PeriodicTaskStatus::stopTask;
    m_task.stop();
  }
}; // class NoOpTableService

class [[nodiscard]] NoOpHistoryService final : public HistoryService {
  // HistoryService interface
  bool isValidHistory(const fs::path& /*dir*/) override { return true; }

  void importHistory(const fs::path& /*historyDir*/,
                     std::function<void()> onProgress = nullptr,
                     std::function<void(std::size_t)> onSetNbFiles = nullptr,
                     std::function<void()> onDone = nullptr) override {
    LOG.debug<__func__>();
    onSetNbFiles(3);
    onProgress();
    onProgress();
    onProgress();
    onDone();
  }
  // use only std::filesystem::path
  void importHistory(auto, std::function<void()>, std::function<void(std::size_t)>,
                     std::function<void()>) = delete;

  void stopImportingHistory() override { LOG.debug<__func__>(); }

  void setHistoryDir(const fs::path& /*dir*/) override {}
}; // NoOpHistoryService

/* no WinMain because we want the console to show debug messages */
/*[[nodiscard]] static inline*/ int main() {
  std::setlocale(LC_ALL, "en_US.utf8");
  LoggingConfig _;
  LOG.debug<"guiDryRun is starting">();
  Database db;
  TableService ts(db);
  HistoryService hs(db);
  auto ret { Gui(ts, hs).run() };
  LOG.debug<"guiDryRun is stopping">();
  return ret;
}
