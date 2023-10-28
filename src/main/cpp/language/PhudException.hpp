#pragma once

#include "strings/StringUtils.hpp" // std::string_view
#include <stdexcept> // std::runtime_error

//
// Note: std::runtime_error comes from <stdexcept> which includes <string>
//
class [[nodiscard]] PhudException : public std::runtime_error {
public:
  explicit PhudException(std::string_view msg);
  virtual ~PhudException() = default;
};
