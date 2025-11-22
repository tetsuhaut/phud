@ECHO OFF
REM got from https://github.com/Microsoft/GSL
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
SET LIB_NAME=GSL
SET SRC_DIR=%~dp0\%LIB_NAME%-4.2.0

IF EXIST %SRC_DIR%\NUL (
  goto :build
)
IF EXIST %SRC_DIR%.tar.gz (
  7z x %SRC_DIR%.tar.gz && 7z x %SRC_DIR%.tar && del %SRC_DIR%.tar && del pax_global_header
  goto :build
)
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  goto :build
)
ECHO Could not find the file %SRC_DIR%.zip neither %SRC_DIR%.tar.gz && EXIT /B 1

:build
SET BUILD_DIR=%~dp0\%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_INSTALL_PREFIX=%BUILD_DIR%/install ^
  -D GSL_TEST=false ^
%SRC_DIR%
ninja install
POPD
ENDLOCAL
@ECHO ON