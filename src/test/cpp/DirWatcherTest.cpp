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
  dw->start([&](const fs::path& file) {
    std::lock_guard lock {mutex};
    changedFiles.push_back(file.stem().string());
    cv.notify_one();
  });
  auto tb = TimeBomb::create(TB_PERIOD, "DirWatcherTest_DetectingChangedFilesShouldWork");

  // With EFSW (event-driven), modifications are detected immediately
  // Add a small delay between modifications to ensure they are processed
  tmpFile.print("yip");
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  tmpFile.print("yip");
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  tmpFile.print("yip");

  // Attendre avec un prédicat et un timeout
  {
    std::unique_lock lock {mutex};
    cv.wait_for(lock, std::chrono::seconds(2), [&]() {
      return !changedFiles.empty();
    });
  }

  dw->stop();

  // Accès protégé à changedFiles après stop()
  {
    std::lock_guard lock {mutex};
    BOOST_REQUIRE(!changedFiles.empty());
    BOOST_REQUIRE(tmpFile.path().stem().string() == changedFiles.front());
  }
}

BOOST_AUTO_TEST_SUITE_END()
