#include "language/PhudException.hpp" // StringView, std::string

#if defined(__MINGW32__) // removal of specific gcc warnings due to Boost
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Weffc++"
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#endif  // __MINGW32__

#include <boost/stacktrace.hpp>

#if defined(__MINGW32__) // end of specific gcc warnings removal
#  pragma GCC diagnostic pop
#endif  // _MSC_VER

#include <sstream> // std::ostringstream

[[nodiscard]] static inline std::string buildMsg(StringView msg) {
  std::ostringstream oss;
  oss << '\n' << msg << " at " << boost::stacktrace::stacktrace() << '\n';
  return oss.str();
}

PhudException::PhudException(StringView msg) : std::runtime_error(buildMsg(msg)) {}