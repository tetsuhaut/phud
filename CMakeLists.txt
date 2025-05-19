cmake_minimum_required(VERSION 3.14.4)

project(Poker_Head-Up_Display
 VERSION 0.0.2
 LANGUAGES CXX
)

# uncomment in case we want to know where CMake is looking for packages
#set(CMAKE_FIND_DEBUG_MODE 1)

# building in-tree is not allowed
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree built. Remove CMakeCache.txt, the CMakeFiles directory and create a build directory outside of the source code and call cmake from there.")
endif()

# set build type, "Debug" by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "build type (one of Debug, RelWithDebInfo, Release, MinSizeRel)" FORCE)
endif()

# use the UTF-8 encoding internally
option(CMAKE_ENCODING_UTF8 "Use UTF-8 encoding internally." ON)

# use C++ 23
# target_compile_features(lib PRIVATE c++ compile feature) ?
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# where to produce executables and libraries
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

# build the mainLib main library with all the src/main/cpp files excepted those containing entry points
file(GLOB_RECURSE mainLibSourceFiles src/main/cpp/*)
list(REMOVE_ITEM mainLibSourceFiles ${CMAKE_SOURCE_DIR}/src/main/cpp/phud/phud.cpp)
list(REMOVE_ITEM mainLibSourceFiles ${CMAKE_SOURCE_DIR}/src/main/cpp/dbgen/dbgen.cpp)
list(REMOVE_ITEM mainLibSourceFiles ${CMAKE_SOURCE_DIR}/src/main/cpp/guiDryRun/guiDryRun.cpp)
add_library(mainLib ${mainLibSourceFiles})
target_include_directories(mainLib PRIVATE src/main/cpp)

# build the phud GUI executable, it uses the main lib
file(GLOB_RECURSE phudSourceFiles ${CMAKE_SOURCE_DIR}/src/main/cpp/phud/*)
# if on Windows, do no show console
add_executable(phud WIN32 ${phudSourceFiles})
target_include_directories(phud PRIVATE src/main/cpp)
target_link_libraries(phud PRIVATE mainLib)

# build the dbgen database generator executable, it uses the main lib
file(GLOB_RECURSE dbgenSourceFiles src/main/cpp/dbgen/*)
add_executable(dbgen ${dbgenSourceFiles})
target_include_directories(dbgen PRIVATE src/main/cpp)
target_link_libraries(dbgen PRIVATE mainLib)

# build the guiDryRun executable, it uses the main lib
file(GLOB_RECURSE guiDryRunSourceFiles src/main/cpp/guiDryRun/*)
add_executable(guiDryRun ${guiDryRunSourceFiles})
target_include_directories(guiDryRun PRIVATE src/main/cpp)
target_link_libraries(guiDryRun PRIVATE mainLib)

# build the unitTests executable, it uses the main lib
file(GLOB_RECURSE testSourceFiles src/test/cpp/*)
add_executable(unitTests ${testSourceFiles})
target_include_directories(unitTests PRIVATE src/main/cpp)
target_link_libraries(unitTests PRIVATE mainLib)

# pass informations to the source code
target_compile_definitions(mainLib PUBLIC PHUD_APP_VERSION="${CMAKE_PROJECT_VERSION}")
target_compile_definitions(mainLib PUBLIC PHUD_APP_NAME_SHORT="phud")
target_compile_definitions(mainLib PUBLIC PHUD_APP_NAME_LONG="Poker Head-Up Display")
target_compile_definitions(unitTests PRIVATE PHUD_MAIN_SRC_DIR="${CMAKE_SOURCE_DIR}/src/main/cpp/")
target_compile_definitions(unitTests PRIVATE PHUD_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/src/test/resources/")
target_compile_definitions(unitTests PRIVATE PHUD_TEST_SRC_DIR="${CMAKE_SOURCE_DIR}/src/test/cpp/")

################################################################################
# libraries configuration
################################################################################

################################################################################
# will use the Boost libraries, so configure the project for it
# need to have Boost_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/boostorg/test
################################################################################
# disable auto linking
add_definitions(-DBOOST_ALL_NO_LIB)

# use static linking, requires the IDE project to use static runtime
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

# specify which libraries to link with and corresponding include files
find_package(Boost 1.80.0 REQUIRED COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(unitTests PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

################################################################################
# will use the FLTK libraries, so configure the project for it
# need to have FLTK_DIR as an environment variable pointing to the dir where
# the library has been installed
# notes:
# - CMake's findFltk tool does not point on the installed version, so can't use it
# - FLTK cmake conf does not handle version :(
# https://github.com/fltk/fltk
################################################################################
set(FLTK_SKIP_FORMS true)
set(FLTK_SKIP_FLUID true)
set(FLTK_SKIP_IMAGES true)
set(FLTK_SKIP_OPENGL true)
find_package(FLTK REQUIRED) # dunno how to provide version
if(NOT (FLTK_VERSION GREATER_EQUAL 1.3.7))
  message(FATAL_ERROR "Wrong FLTK version '${FLTK_VERSION}'")
endif()
target_include_directories(mainLib PRIVATE ${FLTK_INCLUDE_DIRS})
target_link_libraries(mainLib PRIVATE ${FLTK_LIBRARIES})

################################################################################
# will use the frozen library, so configure the project for it
# need to have frozen_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/serge-sans-paille/frozen
################################################################################
find_package(frozen 1.1.0 REQUIRED)
target_link_libraries(mainLib PRIVATE frozen::frozen-headers)
target_link_libraries(unitTests PRIVATE frozen::frozen-headers)

################################################################################
# will use the GSL library, so configure the project for it
# need to have Microsoft.GSL_DIR as an environment variable pointing to the dir
# where the library has been installed
# https://github.com/microsoft/GSL
################################################################################
find_package(Microsoft.GSL CONFIG)
target_link_libraries(mainLib PRIVATE Microsoft.GSL::GSL)
target_link_libraries(phud PRIVATE Microsoft.GSL::GSL)
target_link_libraries(unitTests PRIVATE Microsoft.GSL::GSL)

################################################################################
# will use the SQLite library, so configure the project for it
# no way to specify the project version :(
# need to have SQLite3_DIR as an environment variable pointing to the dir where
# the file Sqlite3Config.cmake has been generated
# the CONFIG is used to comply the website explanations
# https://github.com/azadkuh/sqlite-amalgamation
################################################################################
find_package(SQLite3 REQUIRED CONFIG)
target_link_libraries(mainLib PRIVATE SQLite::SQLite3)

################################################################################
# will use the spdlog library, so configure the project for it
# need to have Spdlog_DIR as an environment variable pointing to the dir where
# the library has been installed.
# Note: we'll use fmt from this lib.
# https://github.com/gabime/spdlog
################################################################################
find_package(spdlog 1.12.0 REQUIRED)
target_link_libraries(dbgen PRIVATE spdlog::spdlog)
target_link_libraries(guiDryRun PRIVATE spdlog::spdlog)
target_link_libraries(mainLib PRIVATE spdlog::spdlog)
target_link_libraries(phud PRIVATE spdlog::spdlog)
target_link_libraries(unitTests PRIVATE spdlog::spdlog)

################################################################################
# will use the stlab library, so configure the project for it
# need to have stlab_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/stlab/libraries
################################################################################
find_package(stlab 1.7.1 REQUIRED)
target_link_libraries(mainLib PRIVATE stlab::stlab)
target_link_libraries(unitTests PRIVATE stlab::stlab)

################################################################################
# will use the utf8cpp library, so configure the project for it
# need to have utf8cpp_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/nemtrif/utfcpp
################################################################################
find_package(utf8cpp REQUIRED) # how to specify the version?
target_link_libraries(unitTests PRIVATE utf8cpp)

################################################################################
# if used with Microsoft Visual Studio, link statically, compile with max
# warnings, compile with multiple threads (i.e. faster), use utf-8 without byte
# order mark, use phud as startup project, use the incremental build.
# see https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
################################################################################
if(MSVC)
  set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO
  )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" TMP "${${CompilerFlag}}")
    set(${CompilerFlag} "${TMP}" CACHE STRING "" FORCE)
    string(REGEX REPLACE "/W[0-4]" "/W4 /w35038 /w14640 /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906 /w14928" TMP "${${CompilerFlag}}")
    set(${CompilerFlag} "${TMP}" CACHE STRING "" FORCE)
  endforeach()
  set_target_properties(dbgen PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)
  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT phud)
  
  # use utf-8 without byte order mark, compile with multiple threads
  # note : /INCREMENTAL is is implied for debug build
  # /fsanitize=address, /fsanitize=undefined, /fsanitize=memory, /fsanitize=thread, /fsanitize=leak, /fsanitize=hwaddress
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP /ZI /Gy /EHsc /Zc:__cplusplus /fsanitize=leak" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP /ZI /Gy /EHsc /Zc:__cplusplus /std:c++23preview " CACHE STRING "" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099" CACHE STRING "" FORCE)

  # target_compile_options(mainLib PRIVATE -fsanitize=address,undefined)
  # target_compile_options(phud PRIVATE -fsanitize=address,undefined)
  # target_compile_options(dbgen PRIVATE -fsanitize=address,undefined)
  # target_compile_options(guiDryRun PRIVATE -fsanitize=address,undefined)
  # target_compile_options(unitTests PRIVATE -fsanitize=address,undefined)
  target_compile_definitions(mainLib PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(phud PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(dbgen PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(guiDryRun PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(unitTests PRIVATE _MSVC_STL_HARDENING=1)

  #target_compile_options(${target_name} PRIVATE -fno-sanitize=signed-integer-overflow)

################################################################################
# if used with MinGW, link statically, compile with max warnings, do not show
# the console window for phud.exe, use the Windows API, the thread API
# Note: if using https://gcc-mcf.lhmouse.com/, the executable must be deployed
# aside of the mcfgthread DLL, because it can't be statically linked. See 
# https://sourceforge.net/p/mingw-w64/mailman/mingw-w64-public/thread/575d13dd.430ac20a.33013.1aa0@mx.google.com/
################################################################################
else()
   # -Werror=conversion breaks compilation because of Boost
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -Wextra -Wpedantic -Werror=unused-function -pedantic-errors -Wvla -Wextra-semi -Wnull-dereference -Wswitch-enum -Wno-deprecated -fno-elide-type -Wduplicated-cond -Wduplicated-branches -Wsuggest-override -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -pedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2 -Weffc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  target_link_libraries(mainLib PRIVATE "-lpsapi -pthread")
  #target_link_libraries(phud PRIVATE -mwindows)
  add_definitions(-DWIN32)
endif(MSVC)
