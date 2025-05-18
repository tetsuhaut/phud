#include "language/PhudException.hpp" // std::string_view, std::string

#include <format>

PhudException::PhudException(std::string_view msg, const std::stacktrace& stacktrace)
: std::runtime_error(std::format("{}\n{}", msg, stacktrace)) {}