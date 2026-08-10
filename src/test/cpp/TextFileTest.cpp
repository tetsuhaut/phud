#include "TestInfrastructure.hpp" // BOOST_* macros, phud::test::*
#include "filesystem/TextFile.hpp"
#include <array>

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(TextFileTest)

BOOST_AUTO_TEST_CASE(TextFileTest_getFileNameAndGetFileStemShouldSucceed) {
  const pt::TmpFile file {"TextFileTest_getFileNameAndGetFileStemShouldSucceed.txt"};
  file.printLn("some content");
  const TextFile tf {file.path()};
  BOOST_REQUIRE("TextFileTest_getFileNameAndGetFileStemShouldSucceed.txt" == tf.getFileName());
  BOOST_REQUIRE("TextFileTest_getFileNameAndGetFileStemShouldSucceed" == tf.getFileStem());
}

BOOST_AUTO_TEST_CASE(TextFileTest_nextAndGetLineAndGetLineIndexShouldIterateOverLines) {
  const pt::TmpFile file {"TextFileTest_nextAndGetLineAndGetLineIndexShouldIterateOverLines.txt"};
  file.printLn("first line");
  file.printLn("");
  file.printLn("third line");
  TextFile tf {file.path()};
  BOOST_REQUIRE(0 == tf.getLineIndex());

  BOOST_REQUIRE(tf.next());
  BOOST_REQUIRE(1 == tf.getLineIndex());
  BOOST_REQUIRE("first line" == tf.getLine());
  BOOST_REQUIRE(!tf.lineIsEmpty());

  BOOST_REQUIRE(tf.next());
  BOOST_REQUIRE(2 == tf.getLineIndex());
  BOOST_REQUIRE(tf.lineIsEmpty());

  BOOST_REQUIRE(tf.next());
  BOOST_REQUIRE(3 == tf.getLineIndex());
  BOOST_REQUIRE("third line" == tf.getLine());
  BOOST_REQUIRE(!tf.lineIsEmpty());

  BOOST_REQUIRE(!tf.next()); // no more lines to read
}

BOOST_AUTO_TEST_CASE(TextFileTest_findShouldReturnAnOptionalPosition) {
  const pt::TmpFile file {"TextFileTest_findShouldReturnAnOptionalPosition.txt"};
  file.printLn("PlayerA collected 150 from pot");
  TextFile tf {file.path()};
  BOOST_REQUIRE(tf.next());

  const auto pos = tf.find(" collected ");
  BOOST_REQUIRE(pos.has_value());
  BOOST_REQUIRE(7 == pos.value());
  BOOST_REQUIRE(!tf.find(" folds").has_value());

  BOOST_REQUIRE(tf.find('P').has_value());
  BOOST_REQUIRE(0 == tf.find('P').value());
  BOOST_REQUIRE(!tf.find('Z').has_value());
}

BOOST_AUTO_TEST_CASE(TextFileTest_containsShouldSucceed) {
  const pt::TmpFile file {"TextFileTest_containsShouldSucceed.txt"};
  file.printLn("PlayerA collected 150 from pot");
  TextFile tf {file.path()};
  BOOST_REQUIRE(tf.next());

  BOOST_REQUIRE(tf.contains(" collected "));
  BOOST_REQUIRE(!tf.contains(" folds"));
  BOOST_REQUIRE(tf.contains('c'));
  BOOST_REQUIRE(!tf.contains('Z'));
}

BOOST_AUTO_TEST_CASE(TextFileTest_startsWithAndEndsWithShouldSucceed) {
  const pt::TmpFile file {"TextFileTest_startsWithAndEndsWithShouldSucceed.txt"};
  file.printLn("*** FLOP *** [Ah Kd Qc]");
  TextFile tf {file.path()};
  BOOST_REQUIRE(tf.next());

  BOOST_REQUIRE(tf.startsWith('*'));
  BOOST_REQUIRE(tf.startsWith("*** FLOP ***"));
  BOOST_REQUIRE(!tf.startsWith("FLOP"));

  BOOST_REQUIRE(tf.endsWith(']'));
  BOOST_REQUIRE(tf.endsWith("Qc]"));
  BOOST_REQUIRE(!tf.endsWith("Kd"));
}

BOOST_AUTO_TEST_CASE(TextFileTest_containsExactShouldOnlyMatchWholeTokens) {
  const pt::TmpFile file {"TextFileTest_containsExactShouldOnlyMatchWholeTokens.txt"};
  file.printLn("sabre_laser folds");
  TextFile tf {file.path()};
  BOOST_REQUIRE(tf.next());

  BOOST_REQUIRE(tf.containsExact("folds"));  // token found at the end of the line
  BOOST_REQUIRE(!tf.containsExact("fold"));  // followed by 's' -> not an exact token
  BOOST_REQUIRE(!tf.containsExact("xyz"));   // not present at all
}

BOOST_AUTO_TEST_CASE(TextFileTest_containsOneOfShouldSucceed) {
  const pt::TmpFile file {"TextFileTest_containsOneOfShouldSucceed.txt"};
  file.printLn("trinita34 raises 42290 to 45490 and is all-in");
  TextFile tf {file.path()};
  BOOST_REQUIRE(tf.next());

  static constexpr std::array<std::string_view, 3> matchingTokens {" folds", " checks", " raises "};
  BOOST_REQUIRE(tf.containsOneOf(matchingTokens));

  static constexpr std::array<std::string_view, 2> nonMatchingTokens {" folds", " checks"};
  BOOST_REQUIRE(!tf.containsOneOf(nonMatchingTokens));
}

BOOST_AUTO_TEST_CASE(TextFileTest_trimShouldRemoveSurroundingWhitespaces) {
  const pt::TmpFile file {"TextFileTest_trimShouldRemoveSurroundingWhitespaces.txt"};
  file.printLn("   padded line   ");
  TextFile tf {file.path()};
  BOOST_REQUIRE(tf.next());
  BOOST_REQUIRE("   padded line   " == tf.getLine());

  tf.trim();
  BOOST_REQUIRE("padded line" == tf.getLine());
}

BOOST_AUTO_TEST_SUITE_END()
