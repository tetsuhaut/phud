#pragma once

#include <stdexcept> // std::runtime_error
#include <string_view>

class [[nodiscard]] PhudException : public std::runtime_error {
public:
  explicit PhudException(std::string_view msg);
  virtual ~PhudException() = default;
};
