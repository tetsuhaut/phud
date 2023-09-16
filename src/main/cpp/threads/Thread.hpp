#pragma once

#include "strings/String.hpp"
#include <thread>

using Thread = std::thread;

String getCurrentThreadId();