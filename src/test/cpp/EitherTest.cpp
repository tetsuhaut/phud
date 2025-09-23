#include "TestInfrastructure.hpp"
#include "language/Either.hpp"

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

BOOST_AUTO_TEST_SUITE_END()
