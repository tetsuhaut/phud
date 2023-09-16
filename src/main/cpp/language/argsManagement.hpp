#pragma once

#include "containers/Tuple.hpp" // std::decay_t, std::forward
#include "system/memory.hpp" // uptr

/**
 * Packages the given arguments into a raw pointer to a Tuple, converted into void*.
 */
[[nodiscard]] static inline void* hideArgs(auto&& ... args) {
  return new Tuple<std::decay_t<decltype(args)>...> { std::forward<decltype(args)>(args)... };
}

/**
 * Converts the given argument to a unique pointer to a Tuple.
 */
template<typename... ARGS>
[[nodiscard]] static inline uptr<Tuple<ARGS...>> revealArgs(void* args) noexcept {
  return uptr<Tuple<ARGS...>>(static_cast<Tuple<ARGS...>*>(args));
}
