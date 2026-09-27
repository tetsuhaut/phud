@ECHO OFF
REM got from https://github.com/stlab/stlab/releases
REM got from https://github.com/stlab/stlab/releases
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
WHERE /q clang && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE /q gcc && (echo ERROR: gcc is in the PATH! && EXIT /B 1)

REM --- stlab-copy-on-write, a dependency of stlab ---
SET LIB_NAME=stlab-copy-on-write
SET SRC_DIR=%LIB_NAME%-1.1.1
IF EXIST %SRC_DIR%\NUL (
  GOTO :build_cow
)
IF EXIST %SRC_DIR%.tar.gz (
  tar -xvzf %LIB_NAME%.tar.gz
  GOTO :build_cow
)
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  GOTO :build_cow
)

ECHO Could not find the file %SRC_DIR%.zip neither %SRC_DIR%.tar.gz && EXIT /B 1

:build_cow
SET BUILD_DIR=stlab-%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_CXX_STANDARD=23 ^
  -D BUILD_TESTING=OFF ^
   --install-prefix %cd%/install ^
  -D CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
  -S ../%SRC_DIR%
ninja
IF ERRORLEVEL 1 (POPD && EXIT /B 1)
POPD
cmake --install %BUILD_DIR%
IF ERRORLEVEL 1 EXIT /B 1

REM --- stlab-enum-ops, a dependency of stlab ---
SET LIB_NAME=stlab-enum-ops
SET SRC_DIR=%LIB_NAME%-1.2.0
IF EXIST %SRC_DIR%\NUL (
  GOTO :build_enum
)
IF EXIST %SRC_DIR%.tar.gz (
  tar -xvzf %LIB_NAME%.tar.gz
  GOTO :build_enum
)
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  GOTO :build_enum
)

ECHO Could not find the file %SRC_DIR%.zip neither %SRC_DIR%.tar.gz && EXIT /B 1

:build_enum
SET BUILD_DIR=stlab-%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_CXX_STANDARD=23 ^
  -D BUILD_TESTING=OFF ^
   --install-prefix %cd%/install ^
  -D CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
  -S ../%SRC_DIR%
ninja
IF ERRORLEVEL 1 (POPD && EXIT /B 1)
POPD
cmake --install %BUILD_DIR%
IF ERRORLEVEL 1 EXIT /B 1

SET LIB_NAME=stlab
SET SRC_DIR=%LIB_NAME%-2.3.0
IF EXIST %SRC_DIR%\NUL (
  GOTO :build
)
IF EXIST %SRC_DIR%.tar.gz (
  tar -xvzf %LIB_NAME%.tar.gz
  GOTO :build
)
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  GOTO :build
)

ECHO Could not find the file %SRC_DIR%.zip neither %SRC_DIR%.tar.gz && EXIT /B 1
GOTO :end

:build
SET BUILD_DIR=stlab-%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_CXX_STANDARD=23 ^
  -D BUILD_TESTING=OFF ^
   --install-prefix %cd%/%BUILD_DIR%/install ^
  -D CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
  -S ../%SRC_DIR%
ninja
POPD
cmake --install %BUILD_DIR%

ECHO.
ECHO Done. Set these environment variables before generating phud:
ECHO   stlab_DIR=%cd%\%BUILD_DIR%\%BUILD_DIR%\install\lib\cmake\stlab
ECHO   stlab-copy-on-write_DIR=%cd%\stlab-stlab-copy-on-write-build-%COMPILER%\install\lib\cmake\stlab-copy-on-write
ECHO   stlab-enum-ops_DIR=%cd%\stlab-stlab-enum-ops-build-%COMPILER%\install\lib\cmake\stlab-enum-ops

:end
POPD
ENDLOCAL
@ECHO ON