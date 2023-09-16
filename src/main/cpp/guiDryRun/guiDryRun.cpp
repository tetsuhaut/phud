#include "containers/ThreadSafeQueue.hpp" // uptr
#include "gui/Gui.hpp"
#include "log/Logger.hpp" // CURRENT_FILE_NAME
#include "mainLib/AppInterface.hpp" // StringView, std::function
#include "mainLib/ProgramInfos.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/PeriodicTask.hpp"

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

  void importHistory(const Path& /*historyDir*/,
                     FunctionVoid incrementCb = nullptr,
                     FunctionInt setNbFilesCb = nullptr,
                     FunctionVoid doneCb = nullptr) override {
    LOG.debug<__func__>();
    setNbFilesCb(3);
    incrementCb();
    incrementCb();
    incrementCb();
    doneCb();
  }
  // use only Path
  void importHistory(auto, FunctionVoid, FunctionInt, FunctionVoid) = delete;

  void stopImportingHistory() override { LOG.debug<__func__>(); }

  String startProducingStats(StringView /*table*/, std::function<void(TableStatistics&& ts)> /*observer*/) override {
    LOG.debug<__func__>();
    m_continue = true;
    m_task.start([this]() {
      LOG.debug<"task in guiDryRun startProducingStats()">();
      std::array<uptr<PlayerStatistics>, 10> fakeStats{
        mkUptr<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player0", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = true,  .nbHands = 10, .vpip = 1, .pfr = 7 }),
        mkUptr<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player1", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 20, .vpip = 2, .pfr = 8 }),
        mkUptr<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player2", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 30, .vpip = 3, .pfr = 9 }),
        mkUptr<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player3", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 40, .vpip = 4, .pfr = 10}),
        mkUptr<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player4", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 50, .vpip = 5, .pfr = 11}),
        mkUptr<PlayerStatistics>(PlayerStatistics::Params {.playerName = "player5", .siteName = ProgramInfos::WINAMAX_SITE_NAME, .isHero = false, .nbHands = 60, .vpip = 6, .pfr = 12}),
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

  void setHistoryDir(const Path& /*dir*/) override {}
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
