#pragma once

#include "strings/StringView.hpp"

namespace ProgramInfos {

static constexpr StringView APP_VERSION {
#ifndef PHUD_APP_VERSION
#error The macro PHUD_APP_VERSION should have been defined in CMakeLists.txt
#endif // PHUD_APP_VERSION
  PHUD_APP_VERSION
};

static constexpr StringView APP_SHORT_NAME {
#ifndef PHUD_APP_NAME_SHORT
#error The macro PHUD_APP_NAME_SHORT should have been defined in CMakeLists.txt
#endif // PHUD_APP_NAME_SHORT
  PHUD_APP_NAME_SHORT
};

static constexpr StringView APP_LONG_NAME {
#ifndef PHUD_APP_NAME_LONG
#error The macro PHUD_APP_NAME_LONG should have been defined in CMakeLists.txt
#endif // PHUD_APP_NAME_LONG
  PHUD_APP_NAME_LONG
};

static constexpr StringView APP_LONG_NAME_AND_VERSION {
  PHUD_APP_NAME_LONG " - Version " PHUD_APP_VERSION
};

static constexpr StringView WINAMAX_EXECUTABLE_STEM {
  "Winamax Poker"
};

static constexpr StringView WINAMAX_SITE_NAME {
  "Winamax"
};

static constexpr StringView PMU_EXECUTABLE_STEM {
  "PMU"
};

static constexpr StringView PMU_SITE_NAME {
  "Pmu"
};

static constexpr StringView POKER_SITE_EXECUTABLE_STEMS[] {
  WINAMAX_EXECUTABLE_STEM, PMU_EXECUTABLE_STEM
};
} // ProgramInfos

#undef PHUD_APP_VERSION
#undef PHUD_APP_NAME_SHORT
#undef PHUD_APP_NAME_LONG