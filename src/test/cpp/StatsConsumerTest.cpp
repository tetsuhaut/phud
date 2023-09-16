#include "TestInfrastructure.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsConsumer.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ConditionVariable.hpp" // Mutex

BOOST_AUTO_TEST_SUITE(StatsReaderTest)

BOOST_AUTO_TEST_CASE(StatsReaderTest_notifyingShouldSucceed) {
  ThreadSafeQueue<TableStatistics> queue;
  StatsConsumer consumer { std::chrono::milliseconds(1), queue};
  std::array<uptr<PlayerStatistics>, 10> playerStats {
    mkUptr<PlayerStatistics>(PlayerStatistics::Params {
      .playerName = "player1",
      .siteName = "someSite", .isHero = true, .nbHands = 1, .vpip = 1, .pfr = 1 }),
    mkUptr<PlayerStatistics>(PlayerStatistics::Params {
      .playerName = "player2",
      .siteName = "someSite", .isHero = false, .nbHands = 2, .vpip = 2, .pfr = 2 }),
    mkUptr<PlayerStatistics>(PlayerStatistics::Params {
      .playerName = "player3",
      .siteName = "someSite", .isHero = false, .nbHands = 3, .vpip = 3, .pfr = 3 }),
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
  };
  queue.push({ .m_maxSeats = Seat::seatThree, .m_tableStats = std::move(playerStats) });
  int nbNotified = 0;
  ConditionVariable cv;
  consumer.consumeAndNotify([&nbNotified, &cv](TableStatistics&) {
    nbNotified++;
    cv.notify_one();
  });
  Mutex mutex;
  {
    UniqueLock lock { mutex };
    cv.wait(lock);
  }
  BOOST_REQUIRE(0 != nbNotified);
  BOOST_REQUIRE(queue.isEmpty());
}

BOOST_AUTO_TEST_SUITE_END()