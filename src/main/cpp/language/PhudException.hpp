#pragma once

#include <stdexcept> // std::runtime_error
#include <string_view>

#if defined(__MINGW32__) // removal of specific gcc warnings due to Boost
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Weffc++"
#endif  // __MINGW32__

#include <boost/stacktrace.hpp> // as std::stacktrace does not work with gcc 15.2 on Windows

#if defined(__MINGW32__) // end of specific gcc warnings removal
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

class [[nodiscard]] PhudException : public std::runtime_error {
public:
  explicit PhudException(std::string_view msg, const boost::stacktrace::stacktrace& stacktrace = boost::stacktrace::stacktrace());
  ~PhudException() override = default;
};
