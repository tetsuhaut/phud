#include "TestInfrastructure.hpp"
#include "language/Either.hpp"
#include "db/SqlInsertor.hpp"

BOOST_AUTO_TEST_SUITE(EitherTest)

BOOST_AUTO_TEST_CASE(EitherTest_shouldBeAbleToReturnTwoTypes) {
  const auto& eInt { Either<int, std::string>::left(0) };
  BOOST_REQUIRE(eInt.isLeft());
  BOOST_TEST(!eInt.isRight());
  BOOST_REQUIRE(eInt.getLeft() == 0);
  const auto& eString { Either<int, std::string>::right("someString") };
  BOOST_REQUIRE(!eString.isLeft());
  BOOST_TEST(eString.isRight());
  BOOST_REQUIRE(eString.getRight() == "someString");
}

BOOST_AUTO_TEST_CASE(EitherTest_ErrOrResShouldBeAbleToReturnTwoTypes) {
  const auto& eInt { ErrOrRes<int>::res(0) };
  BOOST_REQUIRE(eInt.isRes());
  BOOST_TEST(!eInt.isErr());
  BOOST_REQUIRE(eInt.getRes() == 0);
  const auto& eString { ErrOrRes<int>::err("someError") };
  BOOST_REQUIRE(!eString.isRes());
  BOOST_TEST(eString.isErr());
  BOOST_REQUIRE(eString.getErr() == "someError");
  const auto& eStringLiteral { ErrOrRes<int>::err<"someError">() };
  BOOST_REQUIRE(!eStringLiteral.isRes());
  BOOST_TEST(eStringLiteral.isErr());
  BOOST_REQUIRE(eStringLiteral.getErr() == "someError");
}

BOOST_AUTO_TEST_SUITE_END()