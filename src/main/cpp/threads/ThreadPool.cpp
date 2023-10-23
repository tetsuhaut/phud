#include "threads/ThreadPool.hpp" // stlab::

void ThreadPool::stop() {
  stlab::pre_exit();
}