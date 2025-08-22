#include "language/PhudException.hpp" // std::string_view, std::string, Stacktrace

PhudException::PhudException(std::string_view msg)
: std::runtime_error(msg.data()) {}