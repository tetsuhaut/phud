@ECHO OFF
REM got from https://github.com/serge-sans-paille/frozen
REM as it is a header only lib, install in Release type
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1

SET LIB_NAME=frozen
SET SRC_DIR=%~dp0%LIB_NAME%-1.2.0
SET ARCHIVE_NAME=%SRC_DIR%.zip
IF EXIST %SRC_DIR% goto :build
IF EXIST %ARCHIVE_NAME% (
  7z x %ARCHIVE_NAME%
  goto :build
) 
ECHO Could not find the file %ARCHIVE_NAME% && EXIT /B 1

:build
SET BUILD_DIR=%~dp0%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Release ^
  -D CMAKE_INSTALL_PREFIX=%BUILD_DIR%/install ^
  -D frozen.tests=OFF ^
%SRC_DIR%
ninja install
POPD
ENDLOCAL
@ECHO ON