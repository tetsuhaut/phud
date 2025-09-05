@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "test" works
where /q devenv.exe || ECHO Could not find the devenv.exe program. Check that it is in the path. && EXIT /B 1
SET CMAKE_GENERATOR="Visual Studio 17 2022" -A x64 -Thost=x64
SET BIN_DIR=bin\Debug\Debug
CALL build.bat
IF ERRORLEVEL 1 ECHO Error creating the phud build configuration for Visual Studio. && EXIT /B 1
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
@ECHO ON