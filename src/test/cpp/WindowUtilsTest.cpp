#include "TestInfrastructure.hpp"
#include "gui/WindowUtils.hpp"
#include "gui/Rectangle.hpp"

BOOST_AUTO_TEST_SUITE(WindowUtilsTest)

BOOST_AUTO_TEST_CASE(WindowUtilsTest_getWindowTitlesShouldNotCrash) {
  // Test que la fonction ne plante pas
  BOOST_CHECK_NO_THROW(std::ignore = getWindowTitles());

  // Test que le résultat est une liste valide (peut être vide)
  auto titles = getWindowTitles();
  BOOST_CHECK(titles.size() != 0);
}

BOOST_AUTO_TEST_SUITE_END()
