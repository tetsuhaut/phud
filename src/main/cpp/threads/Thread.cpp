#include "threads/Thread.hpp"
#include <sstream> // std::ostringstream

String getCurrentThreadId() {
  std::ostringstream oss;
  oss << std::this_thread::get_id();
  return oss.str();
}