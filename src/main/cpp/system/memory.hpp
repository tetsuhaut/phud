#pragma once

#include <memory> // std::unique_ptr, std::make_shared, std::is_constructible_v

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using sptr = std::shared_ptr<T>;

template <typename T, typename... ARGS>
[[nodiscard]] constexpr std::unique_ptr<T> mkUptr(ARGS&& ... args) {
  static_assert(std::is_constructible_v<T, ARGS...>,
                "can't make std::unique_ptr with the given parameters");
  return std::make_unique<T>(std::forward<ARGS>(args)...);
}

template <typename T, typename... ARGS>
[[nodiscard]] constexpr std::shared_ptr<T> mkSptr(ARGS&& ... args) {
  static_assert(std::is_constructible_v<T, ARGS...>,
                "can't make std::shared_ptr with the given parameters");
  return std::make_shared<T>(std::forward<ARGS>(args)...);
}
