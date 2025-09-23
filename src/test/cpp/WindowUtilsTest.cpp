#include "TestInfrastructure.hpp"
#include "gui/WindowUtils.hpp"
#include "gui/Rectangle.hpp"

BOOST_AUTO_TEST_SUITE(WindowUtilsTest)

BOOST_AUTO_TEST_CASE(WindowUtilsTest_toRectangle) {
  RECT winRect{10, 20, 110, 70};
  auto rect = toRectangle(winRect);

  BOOST_CHECK_EQUAL(rect.x, 10);
  BOOST_CHECK_EQUAL(rect.y, 20);
  BOOST_CHECK_EQUAL(rect.w, 100); // right - left = 110 - 10
  BOOST_CHECK_EQUAL(rect.h, 50);  // bottom - top = 70 - 20
}

BOOST_AUTO_TEST_CASE(WindowUtilsTest_toRectangleZeroSize) {
  RECT winRect{10, 20, 10, 20}; // même position pour left/right et top/bottom
  auto rect = toRectangle(winRect);

  BOOST_CHECK_EQUAL(rect.x, 10);
  BOOST_CHECK_EQUAL(rect.y, 20);
  BOOST_CHECK_EQUAL(rect.w, 0);
  BOOST_CHECK_EQUAL(rect.h, 0);
}

BOOST_AUTO_TEST_CASE(WindowUtilsTest_toRectangleNegativeCoordsShouldBePossible) {
  RECT winRect{-10, -20, 90, 80}; // coordonnées négatives possibles
  auto rect = toRectangle(winRect);

  BOOST_CHECK_EQUAL(rect.x, -10);
  BOOST_CHECK_EQUAL(rect.y, -20);
  BOOST_CHECK_EQUAL(rect.w, 100); // 90 - (-10)
  BOOST_CHECK_EQUAL(rect.h, 100); // 80 - (-20)
}

BOOST_AUTO_TEST_CASE(WindowUtilsTest_getLastErrorMessageFromOSShouldNotCrash) {
  // Test que la fonction ne plante pas
  BOOST_CHECK_NO_THROW(std::ignore = getLastErrorMessageFromOS());

  // Test que le résultat n'est pas null
  auto errorMsg = getLastErrorMessageFromOS();
  BOOST_CHECK(errorMsg.data() != nullptr);
}

BOOST_AUTO_TEST_CASE(WindowUtilsTest_getWindowTitlesShouldNotCrash) {
  // Test que la fonction ne plante pas
  BOOST_CHECK_NO_THROW(std::ignore = getWindowTitles());

  // Test que le résultat est une liste valide (peut être vide)
  auto titles = getWindowTitles();
  BOOST_CHECK(titles.size() != 0);
}

BOOST_AUTO_TEST_CASE(WindowUtilsTest_getExecutableNameWithInvalidHandle) {
  // Test avec un handle invalide
  HWND invalidHandle = nullptr;
  auto execName = getExecutableName(invalidHandle);

  // Doit retourner une chaîne vide pour un handle invalide
  BOOST_CHECK(execName.empty());
}

BOOST_AUTO_TEST_CASE(WindowUtilsTest_getTableWindowRectangleWithInvalidName) {
  // Test avec un nom de table inexistant
  auto result = getTableWindowRectangle("NonExistentTableName12345");

  // Doit retourner std::nullopt pour une table inexistante
  BOOST_CHECK(!result.has_value());
}

BOOST_AUTO_TEST_SUITE_END()
