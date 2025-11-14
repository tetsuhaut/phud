@ECHO OFF
REM got from https://github.com/stlab/libraries
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1

SET LIB_NAME=libraries
SET SRC_DIR=%~dp0%LIB_NAME%-2.1.4
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
SET BUILD_DIR=%~dp0\stlab-%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_CXX_STANDARD=23 ^
  -D BUILD_TESTING=OFF ^
   --install-prefix %BUILD_DIR%/install ^
  -D CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
  -S %SRC_DIR%
ninja
cmake --install %BUILD_DIR%

:end
POPD
ENDLOCAL
@ECHO ON