#include "containers/ThreadSafeQueue.hpp" // std::unique_ptr
#include "gui/Gui.hpp"
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/AppInterface.hpp" // std::string_view, std::function
#include "mainLib/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"

namespace fs = std::filesystem;

static Logger LOG { CURRENT_FILE_NAME };

namespace {
struct [[nodiscard]] LoggingConfig final {
  LoggingConfig() { Logger::setupConsoleDebugLogging("[%D %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v"); }
  ~LoggingConfig() { Logger::shutdownLogging(); }
};
} // anonymous namespace

class [[nodiscard]] NoOpApp final : public AppInterface {
private:
  ThreadSafeQueue<TableStatistics> m_stats {};
  bool m_continue { false };
  PeriodicTask m_task;

public:
  NoOpApp() : m_task { std::chrono::milliseconds(500), CURRENT_FILE_NAME } {
    LOG.debug<__func__>();
  }

  void importHistory(const fs::path& /*historyDir*/,
                     std::function<void()> incrementCb = nullptr,
                     std::function<void(std::size_t)> setNbFilesCb = nullptr,
                     std::function<void()> doneCb = nullptr) override {
    LOG.debug<__func__>();
    setNbFilesCb(3);
    incrementCb();
    incrementCb();
    incrementCb();
    doneCb();
  }
  // use only std::filesystem::path
  void importHistory(auto, std::function<void()>, std::function<void(std::size_t)>,
                     std::function<void()>) = delete;

  void stopImportingHistory() override { LOG.debug<__func__>(); }

  std::string startProducingStats(std::string_view /*table*/,
                                  std::function < void(TableStatistics&& ts) > /*observer*/) override {
    LOG.debug<__func__>();
    m_continue = true;
    m_task.start([this]() {
      LOG.debug<"task in guiDryRun startProducingStats()">();
      std::array<std::unique_ptr<PlayerStatistics>, 10> fakeStats{
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player0", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = true,  .nbHands = 10, .vpip = 1, .pfr = 7 }),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player1", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 20, .vpip = 2, .pfr = 8 }),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player2", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 30, .vpip = 3, .pfr = 9 }),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player3", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 40, .vpip = 4, .pfr = 10}),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player4", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 50, .vpip = 5, .pfr = 11}),
        std::make_unique<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player5", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 60, .vpip = 6, .pfr = 12}),
        nullptr, nullptr, nullptr, nullptr
      };
      m_stats.push(std::move(TableStatistics { .m_maxSeats = Seat::seatSix, .m_tableStats = std::move(fakeStats) }));
      LOG.debug<"task in guiDryRun startObservingTable() returns {}">(m_continue);
      return m_continue; // run until the NoOpApp object is destroyed
    });
    return "";
  }

  void stopProducingStats() override {
    LOG.debug<__func__>();
    m_continue = true; m_task.stop();
  }

  [[nodiscard]] int showGui() override {
    LOG.debug<__func__>();
    return Gui(*this).run();
  }

  void setHistoryDir(const fs::path& /*dir*/) override {}
}; // class NoOpApp

/* no WinMain because we want the console to show debug messages */
/*[[nodiscard]] static inline*/ int main() {
  std::setlocale(LC_ALL, "en_US.utf8");
  LoggingConfig _;
  LOG.debug<"guiDryRun is starting">();
  auto ret { NoOpApp().showGui() };
  LOG.debug<"guiDryRun is stopping">();
  return ret;
}
