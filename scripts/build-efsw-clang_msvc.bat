@ECHO OFF
REM got from https://github.com/SpartanJ/efsw
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
where /q 7z || ECHO Could not find the 7z command. Check that it is in the path. && EXIT /B 1
where /q cmake || ECHO Could not find the cmake command. Check that it is in the path. && EXIT /B 1

SET VERSION=master
SET LIB_NAME=efsw
SET SRC_DIR=%LIB_NAME%-%VERSION%
IF EXIST %SRC_DIR%\NUL goto :build
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  goto :build
) ELSE (
  ECHO "Could not find the file %SRC_DIR%.zip nor %SRC_DIR%" && EXIT /B 1
)

:build
SET BUILD_DIR=%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -DBUILD_TEST_APP=OFF ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D BUILD_SHARED_LIBS=OFF ^
  -D BUILD_STATIC_LIBS=ON ^
  -D CMAKE_INSTALL_PREFIX=../%BUILD_DIR%/install ^
  -D CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
..\%SRC_DIR%
ninja && ninja install

ENDLOCAL
@ECHO ON