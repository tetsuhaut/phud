@ECHO OFF
REM got from https://github.com/stlab/stlab/releases
REM stlab depends on stlab-copy-on-write, stlab-enum-ops and stlab-execution: they are built and
REM installed first, then stlab finds them with CPM_USE_LOCAL_PACKAGES instead of downloading them.
REM CPP_LIBRARY_VERSION gives the version, which cpp-library reads from the git tags (0.0.0 if none).
REM To be launched from the directory containing the source archives.
SETLOCAL

IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
where /q cmake || ECHO Could not find the cmake command. Check that it is in the path. && EXIT /B 1
where /q 7z || ECHO Could not find the 7z command. Check that it is in the path. && EXIT /B 1
where /q gcc || ECHO Could not find the gcc command. Check that it is in the path. && EXIT /B 1
WHERE /q clang && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE /q cl && (echo ERROR: cl is in the PATH! && EXIT /B 1)

SET ROOT_DIR=%CD%
REM the install directories of the libraries already built, given to CMAKE_PREFIX_PATH
SET PREFIX_PATH=

CALL :build_lib stlab-copy-on-write 1.1.1 || EXIT /B 1
CALL :build_lib stlab-enum-ops 1.2.0 || EXIT /B 1
CALL :build_lib stlab-execution 1.0.2 || EXIT /B 1
CALL :build_lib stlab 3.0.1 || EXIT /B 1

SET INSTALL_SUFFIX=build-%COMPILER%\install\lib\cmake
ECHO.
ECHO Done. Set these environment variables before generating phud:
ECHO   stlab_DIR=%ROOT_DIR%\stlab-stlab-%INSTALL_SUFFIX%\stlab
ECHO   stlab-copy-on-write_DIR=%ROOT_DIR%\stlab-stlab-copy-on-write-%INSTALL_SUFFIX%\stlab-copy-on-write
ECHO   stlab-enum-ops_DIR=%ROOT_DIR%\stlab-stlab-enum-ops-%INSTALL_SUFFIX%\stlab-enum-ops
ECHO   stlab-execution_DIR=%ROOT_DIR%\stlab-stlab-execution-%INSTALL_SUFFIX%\stlab-execution
GOTO :end

REM ---------------------------------------------------------------------------
REM Extracts (if needed), builds and installs a library.
REM %1: the library name, %2: its version.
REM The source directory is %1-%2, extracted from %1-%2.zip or %1-%2.tar.gz.
REM ---------------------------------------------------------------------------
:build_lib
SET LIB_NAME=%~1
SET SRC_DIR=%ROOT_DIR%\%LIB_NAME%-%~2
IF NOT EXIST %SRC_DIR%\NUL (
  IF EXIST %SRC_DIR%.tar.gz (
    tar -xvzf %SRC_DIR%.tar.gz
  ) ELSE IF EXIST %SRC_DIR%.zip (
    7z x %SRC_DIR%.zip
  ) ELSE (
    ECHO Could not find the directory %SRC_DIR% nor the file %SRC_DIR%.zip nor %SRC_DIR%.tar.gz
    EXIT /B 1
  )
)

SET BUILD_DIR=%ROOT_DIR%\stlab-%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
cmake ^
  -G "Ninja" ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_CXX_STANDARD=23 ^
  -D BUILD_TESTING=OFF ^
  -D CPP_LIBRARY_VERSION=%~2 ^
  -D CPM_USE_LOCAL_PACKAGES=ON ^
  -D "CMAKE_PREFIX_PATH=%PREFIX_PATH%" ^
  --install-prefix %BUILD_DIR%/install ^
  -S %SRC_DIR% ^
  -B %BUILD_DIR%
IF ERRORLEVEL 1 EXIT /B 1
cmake --build %BUILD_DIR%
IF ERRORLEVEL 1 EXIT /B 1
cmake --install %BUILD_DIR%
IF ERRORLEVEL 1 EXIT /B 1

IF DEFINED PREFIX_PATH (
  SET "PREFIX_PATH=%PREFIX_PATH%;%BUILD_DIR%/install"
) ELSE (
  SET "PREFIX_PATH=%BUILD_DIR%/install"
)
EXIT /B 0

:end
ENDLOCAL
@ECHO ON
