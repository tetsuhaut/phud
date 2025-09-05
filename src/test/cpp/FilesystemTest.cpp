#include "TestInfrastructure.hpp" // BOOST_* macros, phud::test::*
#include "filesystem/TextFile.hpp"

namespace pt = phud::test;
namespace pf = phud::filesystem;

BOOST_AUTO_TEST_SUITE(FilesystemTest)

BOOST_AUTO_TEST_CASE(FilesystemTest_readingFileShouldSucceed) {
  /* phud\src\test\resources\sabre_laser\history\20150630_Super Freeroll Stade 1 - D├®glingos _(123322389)_real_holdem_no-limit.txt */
  const auto& file { pt::getFileFromTestResources(u8"Winamax/sabre_laser/history/20150630_Super Freeroll Stade 1 - Déglingos _(123322389)_real_holdem_no-limit.txt") };
  BOOST_REQUIRE(!pf::isDir(file));
  BOOST_REQUIRE(pf::isFile(file));
  const auto& fileContent { pf::readToString(file) };
  BOOST_REQUIRE(!fileContent.empty());
  BOOST_REQUIRE(!pf::isDir(std::filesystem::path("")));
}

BOOST_AUTO_TEST_CASE(FilesystemTest_listingFileInDirShouldSucceed) {
  pt::TmpDir tmpDir { "FilesystemTest_listingFileInDirShouldSucceed" };
  pt::TmpFile tmpFile { tmpDir / "someTmpFile.txt" };
  tmpFile.print("yop");
  auto files { pf::listTxtFilesInDir(tmpDir.path()) };
  BOOST_REQUIRE(files.size() == 1);
  BOOST_REQUIRE(pf::isFile(files.front()));
}

BOOST_AUTO_TEST_CASE(FilesystemTest_listingSubDirShouldListOnlyDirectories) {
  pt::TmpDir parent { "parent" };
  pt::TmpDir child { parent / "child" };
  pt::TmpFile parentFile { parent / "someTmpFile.txt" };
  parentFile.print("yop");
  BOOST_REQUIRE(pf::isFile(parentFile.path()));
  BOOST_REQUIRE(pf::isDir(parent.path()));
  BOOST_REQUIRE(pf::isDir(child.path()));
  BOOST_REQUIRE(1 == pf::listSubDirs(parent.path()).size());
  BOOST_REQUIRE(pf::listSubDirs(child.path()).empty());
}

BOOST_AUTO_TEST_CASE(TextFileTest_readFileToStringsShouldSucceed) {
  BOOST_TEST(!pf::readToString(
               pt::getFileFromTestResources("Winamax/sabre_laser/history/20141116_Double or Nothing(100679030)_real_holdem_no-limit.txt")).empty());
}

BOOST_AUTO_TEST_CASE(WinamaxGameHistoryTest_parsingHandShouldSucceed) {
  TextFile f{ pt::getFileFromTestResources("Winamax/sabre_laser/history/20141116_Double or "
              "Nothing(100679030)_real_holdem_no-limit.txt") };
  BOOST_REQUIRE(f.lineIsEmpty());
}

BOOST_AUTO_TEST_SUITE_END()