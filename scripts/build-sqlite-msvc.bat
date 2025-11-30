@ECHO OFF
REM got from https://github.com/rhuijben/sqlite-amalgamation
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1

SET LIB_NAME=sqlite
SET SRC_DIR=%LIB_NAME%-amalgamation-3.38.0
IF EXIST %SRC_DIR% goto :build
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  goto :build
)
IF EXIST %SRC_DIR%.tar.gz (
  7z x %SRC_DIR%.tar.gz
  7z x %SRC_DIR%.tar
  DEL %SRC_DIR%.tar
  goto :build
)
ECHO Could not find the archive file %SRC_DIR%.tar.gz
EXIT /B 1

:build
SET BUILD_DIR=%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_INSTALL_PREFIX=../%BUILD_DIR%/install ^
..\%SRC_DIR%
powershell -Command "(gc CMakeCache.txt) -replace '/MD', '/MT' | Out-File -encoding ASCII CMakeCache.txt"
REM powershell -Command "(gc CMakeCache.txt) -replace '/W3', '/W4' | Out-File -encoding ASCII CMakeCache.txt"
ninja install
POPD
ENDLOCAL
@ECHO ON