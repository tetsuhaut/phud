#include "TestInfrastructure.hpp"
#include "language/argsManagement.hpp" // hideArgs, revealArgs, std::is_same_v
#include <atomic> // std::atomic_int
#include <thread>

namespace {
[[nodiscard]] inline int myStaticFunction(int i) { return i; }

struct [[nodiscard]] MyStruct final {
  void myMemberFunction(int i) { m_value = i; }
  int m_value { 0 };
};
};

BOOST_AUTO_TEST_SUITE(ArgsManagementTest)

BOOST_AUTO_TEST_CASE(ArgsManagementTest_hideArgsShouldSucceed) {
  const int anInt { 3 };
  const double aDouble { 4.0 };
  const String aString { "toto" };
  const std::function<int(int)> aFunction { myStaticFunction };
  auto pHidden { hideArgs(anInt, aDouble, aString, aFunction) };
  auto ptr { revealArgs<int, double, String, std::function<int(int)>>(pHidden) };
  const auto [retrievedInt, retrievedDouble, retrievedString, retrievedFctPtr] {*ptr};
  BOOST_REQUIRE(anInt == retrievedInt);
  BOOST_REQUIRE(aDouble == retrievedDouble);
  BOOST_REQUIRE(aString == retrievedString);
  BOOST_REQUIRE(42 == retrievedFctPtr(42));
  static_assert(std::is_same_v<const int, decltype(retrievedInt)>);
  static_assert(std::is_same_v<const double, decltype(retrievedDouble)>);
  static_assert(std::is_same_v<const String, decltype(retrievedString)>);
  static_assert(std::is_same_v<const std::function<int(int)>, decltype(retrievedFctPtr)>);
}

BOOST_AUTO_TEST_CASE(ArgsManagementTest_hideArgsAndRetrieveInAnotherThreadShouldSucceed) {
  const int anInt { 3 };
  const double aDouble { 4.0 };
  const String aString { "toto" };
  const std::function<int(int)> aFunction { myStaticFunction };
  std::atomic_int retrievedFctPtrWasCalled = 0;
  std::thread([&](void* pHidden) {
    auto ptr { revealArgs<int, double, String, std::function<int(int)>>(pHidden) };
    const auto [retrievedInt, retrievedDouble, retrievedString, retrievedFctPtr] {*ptr};
    BOOST_REQUIRE(anInt == retrievedInt);
    BOOST_REQUIRE(aDouble == retrievedDouble);
    BOOST_REQUIRE(aString == retrievedString);
    BOOST_REQUIRE(42 == (retrievedFctPtrWasCalled = retrievedFctPtr(42)));
    static_assert(std::is_same_v<const int, decltype(retrievedInt)>);
    static_assert(std::is_same_v<const double, decltype(retrievedDouble)>);
    static_assert(std::is_same_v<const String, decltype(retrievedString)>);
    static_assert(std::is_same_v<const std::function<int(int)>, decltype(retrievedFctPtr)>);
  },
  hideArgs(anInt, aDouble, aString, aFunction)).join();
  BOOST_REQUIRE(42 == retrievedFctPtrWasCalled.load());
}

BOOST_AUTO_TEST_CASE(ArgsManagementTest_hideMemberFunctionAndRetrieveInAnotherThreadShouldSucceed) {
  MyStruct anObject;
  BOOST_REQUIRE(0 == anObject.m_value);
  const std::function<void(int)> aFunction { [&](int i) { anObject.myMemberFunction(i); } };
  auto myHidden { hideArgs(aFunction) };
  std::thread([&](void* hidden) {
    const auto [retrievedFctPtr] { *revealArgs<std::function<void(int)>>(hidden)} ;
    retrievedFctPtr(42);
    BOOST_REQUIRE(42 == anObject.m_value);
  },
  myHidden).join();
  BOOST_REQUIRE(42 == anObject.m_value); // proves that the lambda was called
}

BOOST_AUTO_TEST_SUITE_END()