IF NOT DEFINED TOOLSET (ECHO call build-boost_<COMPILER>.bat && EXIT /B 1)
IF NOT DEFINED MAKE_EXECUTABLE (ECHO call build-boost_COMPILER.bat && EXIT /B 1)
REM got from https://www.boost.org/
REM try b2.exe --help for help

IF NOT DEFINED COMPILER (ECHO need to define COMPILER && EXIT /B 1)
where /q %MAKE_EXECUTABLE% || ECHO Could not find the %MAKE_EXECUTABLE% command. Check that it is in the path. && EXIT /B 1

SET LIB_NAME=boost
SET SRC_DIR=%~dp0\%LIB_NAME%_1_89_0
IF NOT EXIST %SRC_DIR% (
  IF NOT EXIST %SRC_DIR%.7z ECHO Could not find the file %SRC_DIR%.7z && EXIT /B 1
  7z x %SRC_DIR%.7z
)
SET BUILD_DIR=%~dp0\%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %SRC_DIR%
IF NOT EXIST b2.exe (CALL bootstrap %TOOLSET%)
@ECHO ON
b2 -j8 toolset=%TOOLSET% ^
  address-model=64 ^
  variant=debug ^
  link=static ^
  threading=multi ^
  runtime-link=static ^
  --prefix=%BUILD_DIR% ^
  --build-dir=..\buildTemp ^
  --layout=system ^
  --with-test ^
  --with-stacktrace ^
  --build-type=minimal ^
  install
@ECHO OFF
POPD
RMDIR /Q /S buildTemp