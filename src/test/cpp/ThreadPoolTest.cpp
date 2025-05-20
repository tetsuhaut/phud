#include "TestInfrastructure.hpp"
#include "threads/ThreadPool.hpp"
#include <stlab/concurrency/utility.hpp> // stlab::await

static constexpr double myFunction(double d) { return d / 2; }

BOOST_AUTO_TEST_SUITE(ThreadPoolTest)

BOOST_AUTO_TEST_CASE(ThreadPoolTest_submitFunctionShouldWork) {
  auto futureResult { ThreadPool::submit(myFunction, 3) };
  stlab::await(stlab::copy(futureResult));
  BOOST_REQUIRE(futureResult.is_ready());
  BOOST_REQUIRE(futureResult.get_try().has_value());
  BOOST_REQUIRE(1.5 == futureResult.get_try().value());
}

BOOST_AUTO_TEST_CASE(ThreadPoolTest_submitLambdaWithParametersShouldWork) {
  auto futureResult { ThreadPool::submit([](int d) { return d / d; }, 3) };
  stlab::await(stlab::copy(futureResult));
  BOOST_REQUIRE(futureResult.is_ready());
  BOOST_REQUIRE(futureResult.get_try().has_value());
  BOOST_REQUIRE(1.0 == futureResult.get_try().value());
}

BOOST_AUTO_TEST_CASE(ThreadPoolTest_submitLambdaWithCapturedParametersShouldWork) {
  int d { 3 };
  auto futureResult { ThreadPool::submit([&]() { return d / 2; }) };
  stlab::await(stlab::copy(futureResult));
  BOOST_REQUIRE(futureResult.is_ready());
  BOOST_REQUIRE(futureResult.get_try().has_value());
  BOOST_REQUIRE(1.0 == futureResult.get_try().value());
}

BOOST_AUTO_TEST_SUITE_END()