#include "TestInfrastructure.hpp"
#include "language/ErrOrRes.hpp"

BOOST_AUTO_TEST_SUITE(ErrOrResTest)

BOOST_AUTO_TEST_CASE(ErrOrResTest_ErrOrResShouldBeAbleToReturnTwoTypes) {
  const auto eInt = ErrOrRes<int>::res(0);
  BOOST_REQUIRE(eInt.isRes());
  BOOST_TEST(!eInt.isErr());
  BOOST_REQUIRE(eInt.getRes() == 0);
  const auto eString = ErrOrRes<int>::err("someError");
  BOOST_REQUIRE(!eString.isRes());
  BOOST_TEST(eString.isErr());
  BOOST_REQUIRE(eString.getErr() == "someError");
  const auto eStringLiteral = ErrOrRes<int>::err<"someError">();
  BOOST_REQUIRE(!eStringLiteral.isRes());
  BOOST_TEST(eStringLiteral.isErr());
  BOOST_REQUIRE(eStringLiteral.getErr() == "someError");
}

BOOST_AUTO_TEST_SUITE_END()
