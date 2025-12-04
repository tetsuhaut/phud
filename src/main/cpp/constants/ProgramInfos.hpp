#pragma once

#include <array>
#include <string_view>

namespace ProgramInfos {

static constexpr std::string_view APP_VERSION {
#ifndef PHUD_APP_VERSION
#  error The macro PHUD_APP_VERSION should have been defined in CMakeLists.txt
#endif // PHUD_APP_VERSION
    PHUD_APP_VERSION};

static constexpr std::string_view APP_SHORT_NAME {
#ifndef PHUD_APP_NAME_SHORT
#  error The macro PHUD_APP_NAME_SHORT should have been defined in CMakeLists.txt
#endif // PHUD_APP_NAME_SHORT
    PHUD_APP_NAME_SHORT};

static constexpr std::string_view APP_LONG_NAME {
#ifndef PHUD_APP_NAME_LONG
#  error The macro PHUD_APP_NAME_LONG should have been defined in CMakeLists.txt
#endif // PHUD_APP_NAME_LONG
    PHUD_APP_NAME_LONG};

static constexpr std::string_view APP_LONG_NAME_AND_VERSION {PHUD_APP_NAME_LONG
                                                             " - Version " PHUD_APP_VERSION};

static constexpr std::string_view WINAMAX_EXECUTABLE_STEM {"Winamax Poker"};

static constexpr std::string_view WINAMAX_SITE_NAME {"Winamax"};

static constexpr std::string_view PMU_EXECUTABLE_STEM {"PMU"};

static constexpr std::string_view PMU_SITE_NAME {"Pmu"};

static constexpr std::array<std::string_view, 2> POKER_SITE_EXECUTABLE_STEMS = {WINAMAX_EXECUTABLE_STEM, PMU_EXECUTABLE_STEM};

static constexpr std::string_view DATABASE_NAME {"phud.db"};
} // namespace ProgramInfos

#undef PHUD_APP_VERSION
#undef PHUD_APP_NAME_SHORT
#undef PHUD_APP_NAME_LONG
