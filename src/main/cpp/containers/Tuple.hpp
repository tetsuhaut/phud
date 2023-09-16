#pragma once

//#include <tuple>

//template<typename... Types>
//using Tuple = std::tuple<Types...>;

#include <tuplet/tuple.hpp> // better than std::tuple :(

template<typename... Types>
using Tuple = tuplet::tuple<Types...>;