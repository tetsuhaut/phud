#include "TestInfrastructure.hpp" // String
#include "entities/Action.hpp" // toString(ActionType)
#include "entities/Card.hpp" // toString(Card)
#include "entities/GameType.hpp"
#include "entities/Game.hpp" // toString(GameType)
#include "entities/Hand.hpp" // toString(Street)

namespace ps = phud::strings;

BOOST_AUTO_TEST_SUITE(StringTest)

BOOST_AUTO_TEST_CASE(StringTest_parsingAmountShouldSucceed) {
  BOOST_REQUIRE(10000.0 == ps::toAmount("10000"));
  BOOST_REQUIRE(0.05 == ps::toAmount("0.05€"));
  BOOST_REQUIRE(0.01 == ps::toAmount("0.01€"));
}

BOOST_AUTO_TEST_CASE(StringTest_parsingBuyInShouldSucceed) {
  BOOST_REQUIRE(0 == ps::toBuyIn("Free"));
  BOOST_REQUIRE(5.04 > ps::toBuyIn("4,51 + 0,52"));
  BOOST_REQUIRE(5.02 < ps::toBuyIn("4,51 + 0,52"));
  BOOST_REQUIRE(0.5 == ps::toBuyIn("0,46€ + 0,04€"));
  BOOST_REQUIRE(1.0 == ps::toBuyIn("0,45€ + 0,45€ + 0,10€"));
}

BOOST_AUTO_TEST_CASE(StringTest_containsShouldSucceed) {
  BOOST_REQUIRE(ps::contains("abcdefghijklmnopqrstuvwxyz", 'k'));
  BOOST_REQUIRE(ps::contains("abcdefghijklmnopqrstuvwxyz", "k"));
  BOOST_REQUIRE(ps::contains("abcdefghijklmnopqrstuvwxyz", "ijklmno"));
  BOOST_REQUIRE(ps::contains("abcdefghijklmnopqrstuvwxyz", ""));
  BOOST_TEST(!ps::contains("abcdefghijklmnopqrstuvwxyz", '1'));
  BOOST_TEST(!ps::contains("abcdefghijklmnopqrstuvwxyz", "1"));
  BOOST_TEST(!ps::contains("abcdefghijklmnopqrstuvwxyz", "1234"));
  BOOST_TEST(!ps::contains("", "1234"));
}

BOOST_AUTO_TEST_CASE(StringTest_userTypesToStringShouldWork) {
  BOOST_TEST("check" == toString(ActionType::check));
  BOOST_TEST("Ac" == toString(Card::aceClub));
  BOOST_TEST("no-limit" == toString(Limit::noLimit));
  BOOST_TEST("preflop" == toString(Street::preflop));
  BOOST_TEST("none" == toString(Variant::none));
  BOOST_TEST("tournament" == toString(GameType::tournament));
}

BOOST_AUTO_TEST_CASE(StringTest_trimShouldWork) {
  BOOST_TEST("abc" == ps::trim("abc"));
  BOOST_TEST("abc" == ps::trim("  abc"));
  BOOST_TEST("abc" == ps::trim("abc   "));
  BOOST_TEST("abc" == ps::trim("   abc   "));
}

BOOST_AUTO_TEST_CASE(StringTest_toDoubleTrims) {
  BOOST_REQUIRE(42.0 == ps::toDouble(" 42 "));
  BOOST_REQUIRE(42.0 == ps::toDouble(" 42"));
  BOOST_REQUIRE(42.0 == ps::toDouble("42 "));
}

BOOST_AUTO_TEST_CASE(StringTest_toDoubleShouldWork) {
  BOOST_REQUIRE(0.0 == ps::toDouble("abc"));
  BOOST_REQUIRE(42.0 == ps::toDouble("42"));
  BOOST_REQUIRE(0.0 == ps::toDouble("x42"));
  BOOST_REQUIRE(42.0 == ps::toDouble("42aaa"));
  BOOST_REQUIRE(42.0 == ps::toDouble("42€"));
}

BOOST_AUTO_TEST_CASE(StringTest_replaceShouldWork) {
  BOOST_TEST("" == ps::replaceAll("", 'a', 'b'));
}

namespace {
struct [[nodiscard]] Params final {
  std::string_view value;
}; // struct Params
} // namespace

BOOST_AUTO_TEST_CASE(StringTest_passingStringViewStructAsParamShouldWork) {
  const auto& myFunction {
    [](const Params & p) { return "myFunction returns " + std::string(p.value); }
  };
  BOOST_TEST("myFunction returns a string" == myFunction({.value = "a string"}));
  std::string var { "a string" };
  BOOST_TEST("myFunction returns a string" == myFunction({.value = var}));
  const char arr[] { "a string" };
  BOOST_TEST("myFunction returns a string" == myFunction({.value = arr}));
}


BOOST_AUTO_TEST_SUITE_END()