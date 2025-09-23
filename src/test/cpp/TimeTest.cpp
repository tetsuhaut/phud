#include "TestInfrastructure.hpp"
#include "system/Time.hpp"

BOOST_AUTO_TEST_SUITE(TimeTest)

BOOST_AUTO_TEST_CASE(TimeTest_timeShouldBeInvalidByDefault) {
  BOOST_CHECK_THROW((void)Time({ .strTime = "", .format = WINAMAX_HISTORY_TIME_FORMAT }),
                    TimeException);
  BOOST_CHECK_THROW((void)Time({ .strTime = "", .format = PMU_HISTORY_TIME_FORMAT }), TimeException);
}

BOOST_AUTO_TEST_CASE(TimeTest_timeShouldBeCreatedWithACorrectFormat) {
  BOOST_CHECK_THROW((void)Time({ .strTime = "2014/10/31 00:45:01", .format = PMU_HISTORY_TIME_FORMAT }),
                    TimeException);
  BOOST_CHECK_THROW((void)Time({ .strTime = "Tuesday, September 14, 18:33:39 2021",
                                 .format = WINAMAX_HISTORY_TIME_FORMAT }), TimeException);
}

BOOST_AUTO_TEST_CASE(TimeTest_timeToStringShouldSucceed) {
  const Time time1({ .strTime = "2014/10/31 00:45:01", .format = WINAMAX_HISTORY_TIME_FORMAT });
  BOOST_REQUIRE("2014-10-31 00:45:01" == time1.toSqliteDate());
  const Time time2({ .strTime = "Tuesday, September 14, 18:33:39 2021", .format = PMU_HISTORY_TIME_FORMAT });
  BOOST_REQUIRE("2021-09-14 18:33:39" == time2.toSqliteDate());
}

BOOST_AUTO_TEST_CASE(TimeTest_constructingTimeByCopyShouldSucceed) {
  const Time time1 { Time({.strTime = "2014/10/31 00:45:01", .format = WINAMAX_HISTORY_TIME_FORMAT}) };
  BOOST_REQUIRE("2014-10-31 00:45:01" == time1.toSqliteDate());
  const Time time2 { Time({.strTime = "Tuesday, September 14, 18:33:39 2021", .format = PMU_HISTORY_TIME_FORMAT}) };
  BOOST_REQUIRE("2021-09-14 18:33:39" == time2.toSqliteDate());
}

#include <ctime> // std::tm
#include <iomanip>      // std::get_time
#include <sstream>     // std::istringstream, std::ostream

BOOST_AUTO_TEST_CASE(TimeTest_compilerShouldSupportTimeZoneLol) {
  // %Z is not implemented
  std::tm when {};
  std::istringstream iss { "Tuesday, September 14, 18:33:39 CEST 2021" };
  iss >> std::get_time(&when, "%A, %B %d, %H:%M:%S %Z %Y");
  BOOST_TEST(true == iss.fail());
}

BOOST_AUTO_TEST_SUITE_END()
