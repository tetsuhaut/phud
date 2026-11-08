#include "TestInfrastructure.hpp"
#include "threads/ThreadSafeQueue.hpp"
#include <thread>

BOOST_AUTO_TEST_SUITE(ThreadSafeQueueTest)

BOOST_AUTO_TEST_CASE(ThreadSafeQueueTest_newQueueShouldBeEmpty) {
  ThreadSafeQueue<int> queue;
  BOOST_TEST(queue.isEmpty());
  BOOST_TEST(!queue.tryPop().has_value());
}

BOOST_AUTO_TEST_CASE(ThreadSafeQueueTest_valuesShouldBePoppedInPushOrder) {
  ThreadSafeQueue<int> queue;
  queue.push(1);
  queue.push(2);
  BOOST_TEST(1 == queue.waitPop());
  BOOST_TEST(2 == queue.tryPop().value());
  BOOST_TEST(queue.isEmpty());
}

/**
 * Each thread sleeps in waitPop() until the other one pushes, so a push happens again and again
 * while the other thread is about to sleep. A lost notification would block this test forever.
 */
BOOST_AUTO_TEST_CASE(ThreadSafeQueueTest_waitPopShouldNotMissAPush) {
  static constexpr auto NB_EXCHANGES {300'000};
  ThreadSafeQueue<int> requests;
  ThreadSafeQueue<int> responses;
  std::jthread echo([&requests, &responses]() {
    for (auto i = 0; NB_EXCHANGES > i; ++i) {
      responses.push(requests.waitPop());
    }
  });
  auto nbErrors {0};

  for (auto i = 0; NB_EXCHANGES > i; ++i) {
    requests.push(i);
    nbErrors += (i == responses.waitPop()) ? 0 : 1;
  }

  BOOST_TEST(0 == nbErrors);
}

BOOST_AUTO_TEST_SUITE_END()
