IF NOT DEFINED TOOLSET (ECHO "call build-boost-<COMPILER>.bat" && EXIT /B 1)
IF NOT DEFINED MAKE_EXECUTABLE (ECHO "call build-boost<COMPILER>.bat" && EXIT /B 1)
REM got from https://www.boost.org/
REM try b2.exe --help for help

IF NOT DEFINED COMPILER (ECHO "need to define COMPILER" && EXIT /B 1)
where /q %MAKE_EXECUTABLE% || ECHO Could not find the %MAKE_EXECUTABLE% command. Check that it is in the path. && EXIT /B 1

SET LIB_NAME=boost
SET SRC_DIR=%LIB_NAME%_1_89_0
IF NOT EXIST %SRC_DIR% (
  IF NOT EXIST %SRC_DIR%.7z ECHO "Could not find the file %SRC_DIR%.7z" && EXIT /B 1
  7z x %SRC_DIR%.7z
)
SET BUILD_DIR=%cd%\%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %SRC_DIR%
IF NOT EXIST b2.exe (CALL bootstrap %TOOLSET%)

REM -d0: Turn off all diagnostic output. Only errors are reported.
REM -q: Quit as soon as a build failure is encounteredb
@ECHO ON
b2 -j8 -q -d0 toolset=%TOOLSET% ^
  address-model=64 ^
  architecture=x86 ^
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
if %errorlevel%==0 (
  ECHO installation done
  RMDIR /Q /S ..\buildTemp
)
@ECHO OFF
POPD
