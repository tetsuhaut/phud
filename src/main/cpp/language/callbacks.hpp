#pragma once

#include <functional> // std::function>

using FunctionVoid = std::function<void()>;
using FunctionInt = std::function<void(std::size_t)>;