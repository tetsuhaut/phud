#include "TestInfrastructure.hpp"
#include "TimeBomb.hpp"
#include "filesystem/DirWatcher.hpp"
#include "filesystem/FileUtils.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>

namespace fs = std::filesystem;
namespace pt = phud::test;

static constexpr std::chrono::milliseconds TB_PERIOD {3000};

BOOST_AUTO_TEST_SUITE(DirWatcherTest)

BOOST_AUTO_TEST_CASE(DirWatcherTest_DetectingChangedFilesShouldWork) {
  /* create our own tmp dir to where we watch file changes */
  const pt::TmpDir tmpDir("DirWatcherTest_DetectingChangedFilesShouldWork_tmpDir");
  const pt::TmpFile tmpFile(tmpDir / "someTmpFile.txt");
  tmpFile.print("yop");
  BOOST_REQUIRE(1 == phud::filesystem::listTxtFilesInDir(tmpDir.path()).size());
  const auto dw = DirWatcher::create(tmpDir.path());
  std::vector<std::string> changedFiles;
  std::condition_variable cv;
  std::mutex mutex;
  // each time the file is modified, its name is added to changedFiles
  dw->start([&cv, &mutex, &changedFiles](const fs::path& file) {
    std::lock_guard<std::mutex> lock(mutex);
    changedFiles.push_back(file.stem().string());
    cv.notify_one();
  });
  auto tb = TimeBomb::create(TB_PERIOD, "DirWatcherTest_DetectingChangedFilesShouldWork");

  // Add a small delay between modifications to ensure they are processed
  tmpFile.print("yip");

  // wait until cv is notified, fails after 2 seconds
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(2), [&]() { return !changedFiles.empty(); });
  }

  dw->stop();

  // stop() is now synchronous, so no need to lock after it returns
  BOOST_REQUIRE(!changedFiles.empty());
  BOOST_REQUIRE(tmpFile.path().stem().string() == changedFiles.front());
}

BOOST_AUTO_TEST_SUITE_END()
