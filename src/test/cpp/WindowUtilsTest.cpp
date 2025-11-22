#include "TestInfrastructure.hpp"
#include "gui/WindowUtils.hpp" // mswindows::
#include "gui/Rectangle.hpp"

BOOST_AUTO_TEST_SUITE(WindowUtilsTest)

BOOST_AUTO_TEST_CASE(WindowUtilsTest_getWindowTitlesShouldNotCrash) {
  auto titles = mswindows::getWindowTitles();
  BOOST_CHECK(titles.size() != 0);
}

BOOST_AUTO_TEST_SUITE_END()
