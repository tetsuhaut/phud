#pragma once

#include "strings/StringLiteral.hpp" // std::find_if, std::for_each, etc...
#include <iterator> // std::begin, std::end, std::back_inserter

namespace phud::algorithms {
template<typename CONTAINER, typename FUNCTION>
constexpr void forEach(CONTAINER& c, FUNCTION f) noexcept { std::for_each(std::begin(c), std::end(c), f); }

template<typename CONTAINER, typename FUNCTION>
constexpr void forEach(const CONTAINER& c, FUNCTION f) noexcept { std::for_each(std::begin(c), std::end(c), f); }

template<typename CONTAINER, typename FUNCTION>
[[nodiscard]] constexpr bool anyOf(const CONTAINER& c, FUNCTION f) noexcept { return std::any_of(std::begin(c), std::end(c), f); }

template<typename CONTAINER, typename FUNCTION>
[[nodiscard]] constexpr bool allOf(const CONTAINER& c, FUNCTION f) noexcept { return std::all_of(std::begin(c), std::end(c), f); }

// template<typename CONTAINER, typename FUNCTION>
// [[nodiscard]] bool noneOf(const CONTAINER& c, FUNCTION f) noexcept { return std::none_of(std::begin(c), std::end(c), f); }

// template<typename RET, typename CONTAINER, typename UNARY_OP>
// [[nodiscard]] RET sum(const CONTAINER& c, UNARY_OP f) {
// RET r;
// forEach(c, [&r, &f](const auto & elem) { r += f(elem); });
// return r;
// }

template<typename CONTAINER, typename FUNCTOR>
[[nodiscard]] auto findIf(const CONTAINER& c, FUNCTOR f) {
  return std::find_if(c.begin(), c.end(), f);
}

// template<typename MAP, typename V>
// [[nodiscard]] auto getKeyFromValue(const MAP& m, const V& value) noexcept {
// return std::find_if(std::begin(m), std::end(m), [&value](const auto & pair) {
// return pair.second == value;
// })->first;
// }

template<typename MAP, typename K>
[[nodiscard]] auto getValueFromKey(const MAP& map, const K& key) {
  return map.find(key)->second;
}

/**
 * Returns true if container contains value
 */
template<typename CONTAINER, typename T>
[[nodiscard]] constexpr bool contains(const CONTAINER& c, const T& value) noexcept {
  return std::end(c) != std::find(std::begin(c), std::end(c), value);
}

/**
 * Returns true if container contains VALUE
 */
template<char VALUE>
[[nodiscard]] constexpr bool contains(const auto& container) noexcept {
  return std::end(container) != std::find(std::begin(container), std::end(container), VALUE);
}

/**
 * Returns true if container contains str
 */
template<StringLiteral str>
[[nodiscard]] constexpr bool contains(const auto& container) noexcept {
  return std::end(container) != std::find(std::begin(container), std::end(container), str.value);
}

/**
 * Returns true if c contains a value for which p is true
 */
template<typename CONTAINER, typename PREDICATE>
[[nodiscard]] constexpr bool containsIf(const CONTAINER& c, PREDICATE p) {
  return std::end(c) != std::find_if(std::begin(c), std::end(c), p);
}

//template<typename CONTAINER, typename UNARY_PREDICATE>
//constexpr void removeIf(CONTAINER& c, UNARY_PREDICATE p) {
//  c.erase(std::remove_if(std::begin(c), std::end(c), p), std::end(c));
//}

/**
 * Returns a copy of c, not containing the possible values for which p is true
 */
template<typename CONTAINER_OUTPUT, typename CONTAINER_INPUT, typename UNARY_PREDICATE>
[[nodiscard]] CONTAINER_OUTPUT removeCopyIf(const CONTAINER_INPUT& c, UNARY_PREDICATE p) {
  CONTAINER_OUTPUT ret;
  ret.reserve(c.size());
  std::remove_copy_if(std::begin(c), std::end(c), std::back_inserter(ret), p);
  ret.shrink_to_fit();
  return ret;
}


// works on gcc but not Visual Studio 2019 16.11.3 TODO est-ce toujours d'actualité ?
/*
// use std::back_inserter for types that have push_back
template<typename> struct PM_traits {};
template<typename T, typename U> struct PM_traits<U T::*> { using memberType = U; };

template<typename SOURCE, typename TARGET, typename UNARY_FUNCTION>
constexpr void transform(const SOURCE& source, TARGET& target, UNARY_FUNCTION f) noexcept {
  using MyType = PM_traits<decltype(&TARGET::push_back)>::memberType;

  if constexpr (std::is_function_v<MyType>) {
    std::transform(std::begin(source), std::end(source), std::back_inserter(target), f);
  }
  if constexpr (!std::is_function_v<MyType>) {
    std::transform(std::begin(source), std::end(source), target, f);
  }
}*/

template<typename SOURCE, typename TARGET, typename UNARY_FUNCTION>
constexpr void transform(const SOURCE& source, TARGET& target, UNARY_FUNCTION f) noexcept {
  std::transform(std::begin(source), std::end(source), std::back_inserter(target), f);
}

// template<typename MAP, typename TARGET>
// void copyMapValuesTo(const MAP& m, TARGET& t) noexcept {
// phud::algorithms::transform(m, t, [](auto & entry) { return entry.second; });
// }

// template<typename MAP, typename TARGET>
// void copyMapKeysTo(const MAP& m, TARGET& t) noexcept {
// phud::algorithms::transform(m, t, [](const auto & entry) { return entry.first; });
// }

template<typename SOURCE, typename TARGET>
constexpr void moveInto(SOURCE& s, TARGET& t) noexcept { std::move(std::begin(s), std::end(s), std::back_inserter(t)); }

[[nodiscard]] bool isSet(const auto& container) {
  auto copy { container };
  std::sort(std::begin(copy), std::end(copy));
  return std::end(copy) == std::adjacent_find(std::begin(copy), std::end(copy));
}

[[nodiscard]] constexpr bool isEmpty(const auto& container) { return 0 == std::size(container); }
} // namespace phud::algorithms