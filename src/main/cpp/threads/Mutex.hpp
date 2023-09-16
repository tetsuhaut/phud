#pragma once

#include <mutex>

using Mutex = std::mutex;
using LockGuard = std::lock_guard<std::mutex>;
using UniqueLock = std::unique_lock<std::mutex>;