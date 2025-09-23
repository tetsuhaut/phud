#include "language/PhudException.hpp" // std::string_view, std::string, boost::stacktrace
#include <format>

PhudException::PhudException(std::string_view msg, const boost::stacktrace::stacktrace& stacktrace)
: std::runtime_error(std::format("{}\n{}", msg, boost::stacktrace::to_string(stacktrace))) {}
