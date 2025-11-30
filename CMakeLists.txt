cmake_minimum_required(VERSION 4.1.0)

project(Poker_Head-Up_Display
 VERSION 0.0.2
 LANGUAGES CXX
)

# uncomment in case we want to know where CMake is looking for packages
#set(CMAKE_FIND_DEBUG_MODE 1)

# to generate a file 'compile_commands.json' to be used by include-what-you-use
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

message(STATUS "=== Environment Detection ===")
message(STATUS "WIN32: ${WIN32}")
message(STATUS "MSVC: ${MSVC}")
message(STATUS "MSVC_IDE: ${MSVC_IDE}")
message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
message(STATUS "MINGW: ${MINGW}")
message(STATUS "CYGWIN: ${CYGWIN}")
message(STATUS "UNIX: ${UNIX}")
message(STATUS "APPLE: ${APPLE}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_SIMULATE_ID: ${CMAKE_CXX_SIMULATE_ID}")
message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")

# Note: when using Ninja with MSVC, /showIncludes is required for dependency tracking

if(MSVC)
  message("detected MSVC compiler")
  # link statically with the compiler C runtime
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("detected MSVC + clang compiler")
    add_compile_options()
    add_link_options(--target=x86_64-pc-windows-msvc)
    add_compile_options(--target=x86_64-pc-windows-msvc
                        /W4 # compile with lots of warnings
                        -Wall # most common warnings
                        -Wextra # additionnal warnings
                        -Wpedantic # unnecessary warnings
                        -Weverything # in addition, all of the Clang warnings
                        -Wno-c++98-compat # disable C++98 compatibility warnings
                        -Wno-c++98-compat-pedantic # disable C++98 compatibility warnings
                        -Wno-padded # disable padding warnings as too verbose
                        -Wno-switch-default # disable switch default warning for enums with all cases covered
                        /sdl)
  else()
    add_compile_options(/utf-8 # use UTF-8 for source and execution
                        /MP # compile using multi processor
                        /Gy # the linker will remove unused functions
                        /EHsc # use standard exceptions
                        /Zc:__cplusplus #ask the compiler to fix the __cplusplus macro
                        /guard:cf # enable Control Flow Guard
                        /sdl # enable additional security checks
                        /W4 # compile with lots of warnings
                        /w14242 # implicit conversion
                        /w14254 # type annotation
                        /w14263 # buffer overflow
                        /w14265 # missing virtual destructor
                        /w14287 # unexpected dependencies
                        /w14296
                        /w14311
                        /w14545
                        /w14546
                        /w14547
                        /w14549
                        /w14555
                        /w14619
                        /w14640
                        /w14826
                        /w14905
                        /w14906
                        /w14928
                        /w35038
                        /we4289) # compile warnings configured
  endif()
elseif((CMAKE_CXX_COMPILER_ID MATCHES "Clang")  AND (CMAKE_CXX_SIMULATE_ID STREQUAL "GNU"))
  message("detected GCC + Clang compiler")
  add_compile_options(-Wall # compile with lots of warnings
                      -Wcast-align
                      -Wconversion
                      -Wdouble-promotion
                      -Wduplicated-branches
                      -Wduplicated-cond
                      -Weffc++
                      -Werror=unused-function
                      -Wextra
                      -Wextra-semi
                      -Wformat=2
                      -Wlogical-op
                      -Wmisleading-indentation
                      -Wno-deprecated
                      -Wnon-virtual-dtor
                      -Wnull-dereference
                      -Wold-style-cast
                      -Woverloaded-virtual
                      -Wpedantic
                      -Wshadow
                      -Wsign-conversion
                      -Wsuggest-override
                      -Wswitch-enum
                      -Wno-switch-default
                      -Wunused
                      -Wuseless-cast
                      -Wvla
                      -fexec-charset=UTF-8
                      -finput-charset=UTF-8
                      -fno-elide-type
                      -fsafe-buffer-usage-suggestions
                      -pedantic
                      -pedantic-errors)
elseif(MINGW)
  message("detected GCC MinGW compiler")
  # compiling with GCC MinGW
  # compile with lots of warnings
  add_compile_options(-Wall
                      -Wcast-align
                      -Wconversion
                      -Wdouble-promotion
                      -Wduplicated-branches
                      -Wduplicated-cond
                      -Weffc++
                      -Werror=unused-function
                      -Wextra
                      -Wextra-semi
                      -Wformat=2
                      -Wlogical-op
                      -Wmisleading-indentation
                      -Wno-deprecated
                      -Wnon-virtual-dtor
                      -Wnull-dereference
                      -Wold-style-cast
                      -Woverloaded-virtual
                      -Wpedantic
                      -Wshadow
                      -Wsign-conversion
                      -Wsuggest-override
                      -Wswitch-enum
                      -Wunused
                      -Wuseless-cast
                      -Wvla
                      -fexec-charset=UTF-8
                      -finput-charset=UTF-8
                      -fno-elide-type
                      -pedantic
                      -pedantic-errors
                      -fstack-protector-strong # stack overflow protection
                      -D_FORTIFY_SOURCE=2 # buffer overflow detection
                      -fPIE # Position Independent Executable
                      )
  # Security link options for GCC/MinGW
  add_link_options(-Wl,--dynamicbase # ASLR
                   -Wl,--nxcompat # DEP
                   -pie # Position Independent Executable
                   )
else()
  message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
  set(MY_COMPILER_TYPE "UNKNOWN")
endif()

message("Compiler detection done.")

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
# if on Windows, 'WIN32' makes the program not show console
add_executable(phud WIN32 ${phudSourceFiles})
# add_executable(phud ${phudSourceFiles})
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
# on Windows, avoid interference between min and max macros and std::numeric_limits<>
target_compile_definitions(mainLib PUBLIC NOMINMAX)
target_compile_definitions(unitTests PUBLIC NOMINMAX)
# provide application version number and name
target_compile_definitions(mainLib PUBLIC PHUD_APP_VERSION="${CMAKE_PROJECT_VERSION}")
target_compile_definitions(mainLib PUBLIC PHUD_APP_NAME_SHORT="phud")
target_compile_definitions(mainLib PUBLIC PHUD_APP_NAME_LONG="Poker Head-Up Display")
# tell the test code where the source code is
target_compile_definitions(unitTests PRIVATE PHUD_MAIN_SRC_DIR="${CMAKE_SOURCE_DIR}/src/main/cpp/")
target_compile_definitions(unitTests PRIVATE PHUD_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/src/test/resources/")
target_compile_definitions(unitTests PRIVATE PHUD_TEST_SRC_DIR="${CMAKE_SOURCE_DIR}/src/test/cpp/")

################################################################################
# libraries configuration
################################################################################

################################################################################
# will use the Boost libraries, so configure the project for it
# needs to have Boost_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/boostorg/test
################################################################################
# disable auto linking
add_definitions(-DBOOST_ALL_NO_LIB)

# use static linking, requires the IDE project to use static runtime
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_NO_SYSTEM_PATHS ON)


# specify which libraries to link with and corresponding include files
find_package(Boost CONFIG 1.89.0 REQUIRED COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(unitTests PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

################################################################################
# will use the FLTK libraries, so configure the project for it
# needs to have FLTK_DIR as an environment variable pointing to the dir where
# the library has been installed
# notes:
# - CMake's findFltk tool does not point on the installed version, so can't use it
# https://github.com/fltk/fltk
################################################################################
set(FLTK_SKIP_FORMS true)
set(FLTK_SKIP_FLUID true)
set(FLTK_SKIP_IMAGES true)
set(FLTK_SKIP_OPENGL true)
find_package(FLTK 1.4.4 REQUIRED)
# - FLTK cmake conf does not handle version :(
if(FLTK_VERSION VERSION_LESS "1.4.4")
  message(FATAL_ERROR "FLTK version '${FLTK_VERSION}' is too old")
endif()
target_include_directories(mainLib PRIVATE ${FLTK_INCLUDE_DIRS})
target_link_libraries(mainLib PRIVATE ${FLTK_LIBRARIES})

################################################################################
# will use the frozen library, so configure the project for it
# needs to have frozen_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/serge-sans-paille/frozen
################################################################################
# the version exported by the lib is erroneously 1.1.0 so use it anyway
find_package(frozen 1.1.0 REQUIRED)
target_link_libraries(mainLib PRIVATE frozen::frozen-headers)
target_link_libraries(unitTests PRIVATE frozen::frozen-headers)

################################################################################
# will use the GSL library, so configure the project for it
# needs to have Microsoft.GSL_DIR as an environment variable pointing to the dir
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
# needs to have SQLite3_DIR as an environment variable pointing to the dir where
# the file Sqlite3Config.cmake has been generated
# the CONFIG is used to comply the website explanations
# https://github.com/azadkuh/sqlite-amalgamation
################################################################################
find_package(SQLite3 REQUIRED CONFIG) # see https://github.com/rhuijben/sqlite-amalgamation/issues/8 for getting the version
target_link_libraries(mainLib PRIVATE SQLite::SQLite3)

################################################################################
# will use the spdlog library, so configure the project for it
# needs to have Spdlog_DIR as an environment variable pointing to the dir where
# the library has been installed.
# Note: we'll use fmt from this lib.
# https://github.com/gabime/spdlog
################################################################################
# Force spdlog to use static runtime library
set(SPDLOG_USE_STD_FORMAT OFF)
set(SPDLOG_FMT_EXTERNAL OFF)
find_package(spdlog 1.15.3 REQUIRED)
target_link_libraries(dbgen PRIVATE spdlog::spdlog)
target_link_libraries(guiDryRun PRIVATE spdlog::spdlog)
target_link_libraries(mainLib PRIVATE spdlog::spdlog)
target_link_libraries(phud PRIVATE spdlog::spdlog)
target_link_libraries(unitTests PRIVATE spdlog::spdlog)

################################################################################
# will use the EFSW library, so configure the project for it
# needs to have EFSW_DIR as an environment variable pointing to the dir where
# the library has been installed
# https://github.com/SpartanJ/efsw
################################################################################
find_package(efsw REQUIRED CONFIG)
target_link_libraries(mainLib PRIVATE efsw::efsw)
target_link_libraries(unitTests PRIVATE efsw::efsw)

################################################################################
# will use the stlab library, so configure the project for it
# needs to have stlab_DIR as an environment variable pointing to the dir where
# the stlab 'libraries' library has been installed
# https://github.com/stlab/libraries
################################################################################
# bugged since 2.1.1 :-(
# bug : https://github.com/stlab/libraries/issues/575
# find_package(stlab 2.1.4 REQUIRED)
# target_link_libraries(mainLib PRIVATE stlab::stlab)
# target_link_libraries(unitTests PRIVATE stlab::stlab)

set(STLAB_BUILD_DIR $ENV{stlab_DIR})

if(NOT STLAB_BUILD_DIR)
  message(FATAL_ERROR "The env variable stlab_DIR is missing")
endif()
if(NOT EXISTS "${STLAB_BUILD_DIR}")
  message(FATAL_ERROR "The $stlab_DIR dir does not exist: ${STLAB_BUILD_DIR}")
endif()

set(STLAB_INCLUDE_DIR1 "${STLAB_BUILD_DIR}/include")
set(STLAB_INCLUDE_DIR2 "${STLAB_BUILD_DIR}/../libraries-2.1.4/include")
find_library(STLAB_LIBRARY
    NAMES stlab libstlab
    PATHS "${STLAB_BUILD_DIR}"
    NO_DEFAULT_PATH
)
if(NOT STLAB_LIBRARY)
  message(FATAL_ERROR "Did not find the stlab library in ${STLAB_BUILD_DIR}")
endif()
add_library(stlab STATIC IMPORTED)
set_target_properties(stlab PROPERTIES
    IMPORTED_LOCATION "${STLAB_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${STLAB_INCLUDE_DIR1};${STLAB_INCLUDE_DIR2}"
)
target_link_libraries(mainLib PRIVATE stlab)
target_link_libraries(unitTests PRIVATE stlab)

################################################################################
# will use the utf8cpp library, which is a copy and paste header-only library.
# it should be in src/main/cpp/thirdParties/utfcpp
# https://github.com/nemtrif/utfcpp
################################################################################

################################################################################
# if used with Microsoft Visual Studio, link statically, compile with max
# warnings, compile with multiple threads (i.e. faster), use utf-8 without byte
# order mark, use phud as startup project, use the incremental build.
# see https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
################################################################################
if(MSVC)
  # dbgen writes to console
  target_link_options(dbgen PRIVATE /DEBUG /SUBSYSTEM:CONSOLE)
  # set phud as the Visual Studio startup project
  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT phud)

  # Security hardening for all executables and libraries
  # Control Flow Guard
  add_link_options(/GUARD:CF)
  # Data Execution Prevention (DEP)
  add_link_options(/NXCOMPAT)
  # Address Space Layout Randomization (ASLR)
  add_link_options(/DYNAMICBASE)
  # High entropy ASLR for 64-bit
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_link_options(/HIGHENTROPYVA)
  endif()
  # /fsanitize=address, /fsanitize=undefined, /fsanitize=memory, /fsanitize=thread, /fsanitize=leak, /fsanitize=hwaddress
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP /ZI /Gy /EHsc /Zc:__cplusplus /fsanitize=leak" CACHE STRING "" FORCE)
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099" CACHE STRING "" FORCE)
  # target_compile_options(mainLib PRIVATE -fsanitize=address,undefined)
  # target_compile_options(phud PRIVATE -fsanitize=address,undefined)
  # target_compile_options(dbgen PRIVATE -fsanitize=address,undefined)
  # target_compile_options(guiDryRun PRIVATE -fsanitize=address,undefined)
  # target_compile_options(unitTests PRIVATE -fsanitize=address,undefined)
  #target_compile_options(${target_name} PRIVATE -fno-sanitize=signed-integer-overflow)
  # STL hardening - enhanced runtime checks
  target_compile_definitions(mainLib PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(phud PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(dbgen PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(guiDryRun PRIVATE _MSVC_STL_HARDENING=1)
  target_compile_definitions(unitTests PRIVATE _MSVC_STL_HARDENING=1)

  # Additional security definitions
  # Disable insecure CRT functions warnings (use secure alternatives)
  target_compile_definitions(mainLib PRIVATE _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1)
  target_compile_definitions(mainLib PRIVATE _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1)

  # Enhanced security checks in Debug mode
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Enable iterator debugging
    target_compile_definitions(mainLib PRIVATE _ITERATOR_DEBUG_LEVEL=2)
    target_compile_definitions(phud PRIVATE _ITERATOR_DEBUG_LEVEL=2)
    target_compile_definitions(dbgen PRIVATE _ITERATOR_DEBUG_LEVEL=2)
    target_compile_definitions(guiDryRun PRIVATE _ITERATOR_DEBUG_LEVEL=2)
    target_compile_definitions(unitTests PRIVATE _ITERATOR_DEBUG_LEVEL=2)
  endif()
################################################################################
# if used with MinGW, link statically, compile with max warnings, do not show
# the console window for phud.exe, use the Windows API, the thread API
# Note: if using https://gcc-mcf.lhmouse.com/, the executable must be deployed
# aside of the mcfgthread DLL, because it can't be statically linked. See 
# https://sourceforge.net/p/mingw-w64/mailman/mingw-w64-public/thread/575d13dd.430ac20a.33013.1aa0@mx.google.com/
################################################################################
else()
  target_link_libraries(mainLib PRIVATE "-lpsapi -pthread")
  #target_link_libraries(phud PRIVATE -mwindows)
  add_definitions(-DWIN32)
endif(MSVC)
