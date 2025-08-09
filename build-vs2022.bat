@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "test" works
where /q devenv.exe || ECHO Could not find the devenv.exe program. Check that it is in the path. && EXIT /B 1
SET CMAKE_GENERATOR="Visual Studio 17 2022" -A x64 -Thost=x64
SET BUILD_DIR=build
SET BIN_DIR=bin\Debug\Debug
CALL build.bat
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
IF ERRORLEVEL 1 ECHO Error building the phud Visual Studio project file. && EXIT /B 1
PUSHD %BUILD_DIR%
ENDLOCAL
@ECHO ON