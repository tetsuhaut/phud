#pragma once

#include <stdexcept> // std::runtime_error
#include <string_view>

#if defined(__MINGW32__) // removal of specific compiler warnings due to Boost
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Weffc++"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wcovered-switch-default"
#  pragma clang diagnostic ignored "-Wdeprecated-copy-with-dtor"
#  pragma clang diagnostic ignored "-Wdocumentation"
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#  pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#  pragma clang diagnostic ignored "-Wlanguage-extension-token"
#  pragma clang diagnostic ignored "-Wnested-anon-types"
#  pragma clang diagnostic ignored "-Wnrvo"
#  pragma clang diagnostic ignored "-Wnewline-eof"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wreserved-identifier"
#  pragma clang diagnostic ignored "-Wunused-exception-parameter"
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#  pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif  // __MINGW32__

#include <boost/stacktrace.hpp> // as std::stacktrace does not work with gcc 15.2 on Windows

#if defined(__MINGW32__) // end of specific compiler warnings removal
#  pragma GCC diagnostic pop
#elif defined(__clang__)
#  pragma clang diagnostic pop
#endif  // __MINGW32__

class [[nodiscard]] PhudException : public std::runtime_error {
public:
  explicit PhudException(std::string_view msg, const boost::stacktrace::stacktrace& stacktrace = boost::stacktrace::stacktrace());
  PhudException(const PhudException&) = default;
  PhudException(PhudException&&) = default;
  PhudException& operator=(const PhudException&) = default;
  PhudException& operator=(PhudException&&) = default;
  ~PhudException() override;
};
