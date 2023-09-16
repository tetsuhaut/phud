#pragma once

#include "strings/StringView.hpp" // StringView
#include <stdexcept> // std::runtime_error

//
// Note: std::runtime_error comes from <stdexcept> which includes <string>
//
class [[nodiscard]] PhudException : public std::runtime_error {
public:
  explicit PhudException(StringView msg);
  virtual ~PhudException() = default;
};
