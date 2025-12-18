rem @ECHO OFF
REM got from https://github.com/serge-sans-paille/frozen
REM as it is a header only lib, install in Release type
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
WHERE /q clang && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE /q gcc && (echo ERROR: gcc is in the PATH! && EXIT /B 1)

SET LIB_NAME=frozen
SET SRC_DIR=%LIB_NAME%-1.2.0
SET ARCHIVE_NAME=%SRC_DIR%.zip
IF EXIST %SRC_DIR% goto :build
IF EXIST %ARCHIVE_NAME% (
  7z x %ARCHIVE_NAME%
  goto :build
) 
ECHO Could not find the file %ARCHIVE_NAME% && EXIT /B 1

:build
SET BUILD_DIR=%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Release ^
  -D CMAKE_INSTALL_PREFIX=../%BUILD_DIR%/install ^
  -D frozen.tests=OFF ^
..\%SRC_DIR%
powershell -Command "(gc CMakeCache.txt) -replace '/MD', '/MT' | Out-File -encoding ASCII CMakeCache.txt"
ninja
POPD
cmake --install %BUILD_DIR%
POPD
ENDLOCAL
@ECHO ON