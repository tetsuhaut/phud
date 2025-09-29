#include "TestInfrastructure.hpp"
#include "statistics/PlayerStatistics.hpp"
#include "statistics/StatsConsumer.hpp"
#include "statistics/TableStatistics.hpp"
#include "threads/ThreadSafeQueue.hpp"
#include <condition_variable>
#include <mutex> // std::unique_lock

BOOST_AUTO_TEST_SUITE(StatsReaderTest)

  BOOST_AUTO_TEST_CASE(StatsReaderTest_notifyingShouldSucceed) {
    ThreadSafeQueue<TableStatistics> queue;
    const StatsConsumer consumer { std::chrono::milliseconds(1), queue };
    std::array<std::unique_ptr<PlayerStatistics>, 10> playerStats {
      std::make_unique<PlayerStatistics>(PlayerStatistics::Params {
        .playerName = "player1",
        .siteName = "someSite", .isHero = true, .nbHands = 1, .vpip = 1, .pfr = 1
      }),
      std::make_unique<PlayerStatistics>(PlayerStatistics::Params {
        .playerName = "player2",
        .siteName = "someSite", .isHero = false, .nbHands = 2, .vpip = 2, .pfr = 2
      }),
      std::make_unique<PlayerStatistics>(PlayerStatistics::Params {
        .playerName = "player3",
        .siteName = "someSite", .isHero = false, .nbHands = 3, .vpip = 3, .pfr = 3
      }),
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
    };
    queue.push(TableStatistics { "someSite", "someTable", Seat::seatThree, std::move(playerStats) });
    auto nbNotified { 0 };
    std::condition_variable cv;
    consumer.consumeAndNotify([&nbNotified, &cv](TableStatistics&) {
      nbNotified++;
      cv.notify_one();
    });
    {
      std::mutex mutex;
      std::unique_lock lock { mutex };
      cv.wait(lock);
    }
    BOOST_REQUIRE(0 != nbNotified);
    BOOST_REQUIRE(queue.isEmpty());
  }

BOOST_AUTO_TEST_SUITE_END()
