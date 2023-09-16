#pragma once

#include <vector>

template<typename T>
using Vector = std::vector<T>;

template<template<typename, typename> typename SmartPointer, typename T, typename U>
[[nodiscard]] inline Vector<const T*> mkView(const Vector<SmartPointer<T, U>>& v) {
  Vector<const T*> ret;
  ret.reserve(v.size());

  // here we use a 'for' loop to avoid depending on <algorithm>
  for (const auto& e : v) { ret.push_back(e.get()); }

  return ret;
}
