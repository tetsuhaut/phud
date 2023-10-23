#pragma once

#include <stlab/concurrency/future.hpp> // stlab::async, std::forward
#include <stlab/concurrency/default_executor.hpp>

template<typename  T>
using Future = stlab::future<T>;

namespace ThreadPool {
template<typename F, typename... ARGS>
[[nodiscard]] Future<std::invoke_result_t<F, ARGS...>> submit(F&& f, ARGS&& ... args) {
  return stlab::async(stlab::default_executor, std::forward<F>(f), std::forward<ARGS>(args)...);
}

void stop();
}; // namespace ThreadPool