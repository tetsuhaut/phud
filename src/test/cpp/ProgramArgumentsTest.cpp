#include "TestInfrastructure.hpp"
#include "language/limits.hpp"
#include "phud/ProgramArguments.hpp" // std::filesystem::path, std::vector, std::span

namespace pt = phud::test;

BOOST_AUTO_TEST_SUITE(ProgramArgumentsTest)

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_helpShortParamShouldDisplayHelp) {
  const char* argv[] { "thisProgram.exe", "-h" };
  std::string msg;

  try {
    (void)parseProgramArguments(argv);
  } catch (const UserAskedForHelpException& e) {
    msg = e.what();
  }

  BOOST_REQUIRE(!msg.empty());
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_helpLongParamShouldDisplayHelp) {
  const char* argv[] { "thisProgram.exe", "--help" };
  BOOST_CHECK_THROW((void)parseProgramArguments(argv), UserAskedForHelpException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_unexistingDirShortParamShouldDisplayErrorMessage) {
  const char* argv[] { "thisProgram.exe", "-d", "someUnexistingDir" };
  BOOST_CHECK_THROW((void)parseProgramArguments(argv), ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_unexistingDirLongParamShouldDisplayErrorMessage) {
  const char* argv[] { "thisProgram.exe", "--historyDir", "someUnexistingDir" };
  BOOST_CHECK_THROW((void)parseProgramArguments(argv), ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_unknownParamShouldDisplayErrorMessage) {
  const char* argv[] { "thisProgram.exe", "-x" };
  BOOST_CHECK_THROW((void)parseProgramArguments(argv),  ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_badParamShouldDisplayErrorMessage) {
  const char* argv[] { "thisProgram.exe", "toto" "titi" };
  BOOST_CHECK_THROW((void)parseProgramArguments(argv), ProgramArgumentsException);
}

BOOST_AUTO_TEST_CASE(ProgramArgumentsTest_correctParamShouldSucceed) {
  pt::TmpDir dir { "thisDirExists!!!" };
  const auto& str { dir.string() };
  const char* argv[] {"thisProgram.exe", "-d", str.c_str()};
  const auto& [oWinamaxDir, oLoggingLevel] { parseProgramArguments(argv) };
  BOOST_REQUIRE(dir.path() == oWinamaxDir.value());
  BOOST_REQUIRE(false == oLoggingLevel.has_value());
}

BOOST_AUTO_TEST_SUITE_END()