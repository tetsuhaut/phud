#include "TestInfrastructure.hpp"
#include "phud/ProgramArguments.hpp" // std::filesystem::path, std::vector, std::span

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(ProgramArgumentsTest)

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_helpShortParamShouldDisplayHelp) {
  const char* const argv[] {"thisProgram.exe", "-h"};
  std::string msg;

  try {
    std::ignore = parseProgramArguments(argv);
  } catch (const UserAskedForHelpException& e) {
    msg = e.what();
  }

  BOOST_REQUIRE(!msg.empty());
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_helpLongParamShouldDisplayHelp) {
  const char* const argv[] {"thisProgram.exe", "--help"};
  BOOST_CHECK_THROW(std::ignore = parseProgramArguments(argv), UserAskedForHelpException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_historyDirParamShouldBeUnknown) {
  // the history directory is chosen in the user interface
  const pt::TmpDir dir {"thisDirExists!!!"};
  const auto str = dir.string();
  const char* const argv[] {"thisProgram.exe", "-d", str.c_str()};
  BOOST_CHECK_THROW(std::ignore = parseProgramArguments(argv), ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_unknownParamShouldDisplayErrorMessage) {
  const char* const argv[] {"thisProgram.exe", "-x"};
  BOOST_CHECK_THROW(std::ignore = parseProgramArguments(argv), ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_badParamShouldDisplayErrorMessage) {
  const char* const argv[] {"thisProgram.exe", "toto"
                                         "titi"};
  BOOST_CHECK_THROW(std::ignore = parseProgramArguments(argv), ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_correctParamShouldSucceed) {
  const char* const argv[] {"thisProgram.exe", "--logLevel", "debug"};
  const auto oLoggingLevel = parseProgramArguments(argv);
  BOOST_REQUIRE(oLoggingLevel.has_value());
  BOOST_REQUIRE(LoggingLevel::debug == oLoggingLevel.value());
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_noParamShouldSucceed) {
  const char* const argv[] {"thisProgram.exe"};
  BOOST_REQUIRE(false == parseProgramArguments(argv).has_value());
}

BOOST_AUTO_TEST_SUITE_END()
