#include "TestInfrastructure.hpp"
#include "TimeBomb.hpp"
#include "threads/PeriodicTask.hpp"
#include <array>

namespace {
struct [[nodiscard]] StrContainer final {
  std::string str = "";
}; // struct StrContainer
}; // namespace

static constexpr std::chrono::milliseconds TB_PERIOD { 3000 };
static constexpr std::chrono::milliseconds PT_PERIOD { 50 };

BOOST_AUTO_TEST_SUITE(PeriodicTaskTest)

BOOST_AUTO_TEST_CASE(PeriodicTaskTest_launchingAPeriodicTaskShouldWork) {
  TimeBomb willExplodeIn { TB_PERIOD, "PeriodicTaskTest_launchingAPeriodicTaskShouldWork" };
  PeriodicTask pt { PT_PERIOD };
  std::vector<std::string> v { "yip" };
  pt.start([&]() {
    v.push_back("yop");
    return (4 != v.size());
  });
  pt.join();
  BOOST_REQUIRE(4 == v.size());
}

BOOST_AUTO_TEST_CASE(PeriodicTaskTest_periodicTaskShouldTakeHiddenArgs) {
  TimeBomb willExplodeIn { TB_PERIOD, "PeriodicTaskTest_periodicTaskShouldTakeHiddenArgs" };
  PeriodicTask pt { PT_PERIOD };
  auto pMyStrContainer { mkUptr<StrContainer>() };
  void* hidden { pMyStrContainer.get() };
  pt.start([hidden]() {
    auto pStrContainer { static_cast<StrContainer*>(hidden) };
    pStrContainer->str += "yop";
    return pStrContainer->str != "yopyop";
  });
  pt.join();
  BOOST_REQUIRE("yopyop" == pMyStrContainer->str);
}

struct PassedInArray {
  std::array<sptr<StrContainer>, 2>& m_array;
  PassedInArray(std::array<sptr<StrContainer>, 2>& arr)
    : m_array(arr) {}
};

BOOST_AUTO_TEST_CASE(PeriodicTaskTest_periodicTaskShouldTakeArrays) {
  TimeBomb willExplodeIn { TB_PERIOD, "PeriodicTaskTest_periodicTaskShouldTakeArrays" };
  PeriodicTask pt { PT_PERIOD };
  std::array<sptr<StrContainer>, 2> myArray { mkSptr<StrContainer>(), nullptr };
  void* hidden { new PassedInArray(myArray) };
  pt.start([hidden]() {
    auto args { std::unique_ptr<PassedInArray>(static_cast<PassedInArray*>(hidden)) };
    args->m_array[0]->str = "yop";
    return false;
  });
  pt.join();
  BOOST_REQUIRE("yop" == myArray[0]->str);
  BOOST_REQUIRE(nullptr == myArray[1]);
}

BOOST_AUTO_TEST_SUITE_END()