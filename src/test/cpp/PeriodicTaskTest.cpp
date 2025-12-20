#include "TestInfrastructure.hpp"
#include "TimeBomb.hpp"
#include "threads/PeriodicTask.hpp"
#include <array>

namespace {
  struct [[nodiscard]] StrContainer final {
    std::string str {};
  }; // struct StrContainer
} // anonymous namespace

static constexpr std::chrono::milliseconds TB_PERIOD {3000};
static constexpr std::chrono::milliseconds PT_PERIOD {50};

BOOST_AUTO_TEST_SUITE(PeriodicTaskTest)

BOOST_AUTO_TEST_CASE(PeriodicTaskTest_launchingAPeriodicTaskShouldWork) {
  auto willExplodeIn {
      TimeBomb::create(TB_PERIOD, "PeriodicTaskTest_launchingAPeriodicTaskShouldWork")};
  const PeriodicTask pt {PT_PERIOD};
  std::vector<std::string> v {"yip"};
  pt.start([&v]() {
    v.emplace_back("yop");
    return 4 == v.size() ? PeriodicTaskStatus::stopTask : PeriodicTaskStatus::repeatTask;
  });
  pt.join();
  BOOST_REQUIRE(4 == v.size());
}

BOOST_AUTO_TEST_CASE(PeriodicTaskTest_periodicTaskShouldTakeHiddenArgs) {
  auto willExplodeIn {
      TimeBomb::create(TB_PERIOD, "PeriodicTaskTest_periodicTaskShouldTakeHiddenArgs")};
  const PeriodicTask pt {PT_PERIOD};
  const auto pMyStrContainer {std::make_unique<StrContainer>()};
  void* hidden {pMyStrContainer.get()};
  pt.start([hidden]() {
    const auto pStrContainer {static_cast<StrContainer*>(hidden)};
    pStrContainer->str += "yop";
    return pStrContainer->str == "yopyop" ? PeriodicTaskStatus::stopTask
                                          : PeriodicTaskStatus::repeatTask;
  });
  pt.join();
  BOOST_REQUIRE("yopyop" == pMyStrContainer->str);
}

struct [[nodiscard]] PassedInArray final {
  std::array<std::shared_ptr<StrContainer>, 2>& m_array;

  explicit PassedInArray(std::array<std::shared_ptr<StrContainer>, 2>& arr)
    : m_array(arr) {}
};

BOOST_AUTO_TEST_CASE(PeriodicTaskTest_periodicTaskShouldTakeArrays) {
  auto willExplodeIn {TimeBomb::create(TB_PERIOD, "PeriodicTaskTest_periodicTaskShouldTakeArrays")};
  const PeriodicTask pt {PT_PERIOD};
  std::array<std::shared_ptr<StrContainer>, 2> myArray {std::make_shared<StrContainer>(), nullptr};
  auto pArray = std::make_shared<PassedInArray>(myArray);
  pt.start([pArray]() {
    const auto args = *pArray;
    args.m_array[0]->str = "yop";
    return PeriodicTaskStatus::stopTask;
  });
  pt.join();
  BOOST_REQUIRE("yop" == myArray[0]->str);
  BOOST_REQUIRE(nullptr == myArray[1]);
}

BOOST_AUTO_TEST_SUITE_END()
