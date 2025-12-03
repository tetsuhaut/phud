#include "TestInfrastructure.hpp"
#include "gui/Rectangle.hpp"

BOOST_AUTO_TEST_SUITE(RectangleTest)

BOOST_AUTO_TEST_CASE(RectangleTest_defaultConstruction) {
  constexpr phud::Rectangle rect;
  BOOST_CHECK_EQUAL(rect.x, 0);
  BOOST_CHECK_EQUAL(rect.y, 0);
  BOOST_CHECK_EQUAL(rect.w, 0);
  BOOST_CHECK_EQUAL(rect.h, 0);
}

BOOST_AUTO_TEST_CASE(RectangleTest_valueConstruction) {
  constexpr phud::Rectangle rect {10, 20, 100, 50};
  BOOST_CHECK_EQUAL(rect.x, 10);
  BOOST_CHECK_EQUAL(rect.y, 20);
  BOOST_CHECK_EQUAL(rect.w, 100);
  BOOST_CHECK_EQUAL(rect.h, 50);
}

BOOST_AUTO_TEST_CASE(RectangleTest_copyConstruction) {
  constexpr phud::Rectangle rect1 {10, 20, 100, 50};
  const auto [x, y, w, h] {rect1};

  BOOST_CHECK_EQUAL(x, rect1.x);
  BOOST_CHECK_EQUAL(y, rect1.y);
  BOOST_CHECK_EQUAL(w, rect1.w);
  BOOST_CHECK_EQUAL(h, rect1.h);
}

BOOST_AUTO_TEST_CASE(RectangleTest_assignment) {
  constexpr phud::Rectangle rect1 {10, 20, 100, 50};
  const auto [x, y, w, h] {rect1};
  BOOST_CHECK_EQUAL(x, rect1.x);
  BOOST_CHECK_EQUAL(y, rect1.y);
  BOOST_CHECK_EQUAL(w, rect1.w);
  BOOST_CHECK_EQUAL(h, rect1.h);
}

BOOST_AUTO_TEST_CASE(RectangleTest_structuredBinding) {
  phud::Rectangle rect {10, 20, 100, 50};
  auto [x, y, w, h] = rect;

  BOOST_CHECK_EQUAL(x, 10);
  BOOST_CHECK_EQUAL(y, 20);
  BOOST_CHECK_EQUAL(w, 100);
  BOOST_CHECK_EQUAL(h, 50);
}

BOOST_AUTO_TEST_CASE(RectangleTest_negativeValues) {
  constexpr phud::Rectangle rect {-10, -20, 100, 50};

  BOOST_CHECK_EQUAL(rect.x, -10);
  BOOST_CHECK_EQUAL(rect.y, -20);
  BOOST_CHECK_EQUAL(rect.w, 100);
  BOOST_CHECK_EQUAL(rect.h, 50);
}

BOOST_AUTO_TEST_SUITE_END()
