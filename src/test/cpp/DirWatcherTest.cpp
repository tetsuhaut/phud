#include "TestInfrastructure.hpp"
#include "TimeBomb.hpp"
#include "filesystem/DirWatcher.hpp"
#include "filesystem/FileUtils.hpp"
#include <condition_variable>
#include <mutex>

namespace fs = std::filesystem;
namespace pt = phud::test;

static constexpr std::chrono::milliseconds TB_PERIOD { 3000 };
static constexpr std::chrono::milliseconds WATCH_PERIOD { 50 };

BOOST_AUTO_TEST_SUITE(DirWatcherTest)

BOOST_AUTO_TEST_CASE(DirWatcherTest_DetectingChangedFilesShouldWork) {
  /* create our own tmp dir to where we watch file changes */
  const pt::TmpDir tmpDir { "DirWatcherTest_DetectingChangedFilesShouldWork_tmpDir" };
  const pt::TmpFile tmpFile { tmpDir / "someTmpFile.txt" };
  tmpFile.print("yop");
  BOOST_REQUIRE(1 == phud::filesystem::listTxtFilesInDir(tmpDir.path()).size());
  DirWatcher dw { WATCH_PERIOD, tmpDir.path() };
  std::vector<std::string> changedFiles;
  std::condition_variable cv;
  dw.start([&](const fs::path & file) {
    changedFiles.push_back(file.stem().string());
    cv.notify_one();
  });
  TimeBomb _ { TB_PERIOD, "DirWatcherTest_DetectingChangedFilesShouldWork" };
  tmpFile.print("yip");
  tmpFile.print("yip");
  tmpFile.print("yip");
  std::mutex mutex;
  {
    std::unique_lock<std::mutex> lock { mutex };
    cv.wait(lock);
  }
  dw.stop();
  BOOST_REQUIRE(!changedFiles.empty());
  BOOST_REQUIRE(tmpFile.path().stem().string() == changedFiles.front());
}

BOOST_AUTO_TEST_SUITE_END()
