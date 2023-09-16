#pragma once

#include <system_error>
using ErrorCode = std::error_code;

[[nodiscard]] inline bool isOk(const ErrorCode& ec) noexcept { return 0 == ec.value(); }