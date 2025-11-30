@ECHO OFF
REM got from https://github.com/gabime/spdlog
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
where /q 7z || ECHO Could not find the 7z command. Check that it is in the path. && EXIT /B 1
where /q cmake || ECHO Could not find the cmake command. Check that it is in the path. && EXIT /B 1

SET VERSION=1.15.3
SET LIB_NAME=spdlog
SET SRC_DIR=%LIB_NAME%-%VERSION%
IF EXIST %SRC_DIR%\NUL goto :build
IF EXIST %SRC_DIR%.tar.gz (
  tar -xvzf %SRC_DIR%.tar.gz
  goto :build
) ELSE (
  ECHO Could not find the archive file %SRC_DIR%.tar.gz
)
IF EXIST %SRC_DIR%.zip (
  7z x %SRC_DIR%.zip
  goto :build
) ELSE (
  ECHO "Could not find the file %SRC_DIR%.zip nor %SRC_DIR%.tar.gz" && EXIT /B 1
)

:build
SET BUILD_DIR=%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D SPDLOG_BUILD_SHARED=OFF ^
  -D SPDLOG_FMT_EXTERNAL=OFF ^
  -D SPDLOG_BUILD_EXAMPLE=OFF ^
  -D SPDLOG_BUILD_TESTS=OFF ^
  -D CMAKE_INSTALL_PREFIX=../%BUILD_DIR%/install ^
  -D CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
..\%SRC_DIR%
ninja && ninja install
POPD
ENDLOCAL

REM to ensure the compiled lib is static:
REM lib /list spdlog.lib
REM will show a list of .obj files that are inside the spdlog.lib file

@ECHO ON