@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "test" works

WHERE /q gcc || ECHO Could not find the gcc command. Check that it is in the path. && EXIT /B 1
WHERE /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
WHERE clang >nul 2>&1 && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE /q cl && (echo ERROR: cl is in the PATH! && EXIT /B 1)

SET CMAKE_GENERATOR="Ninja"
SET BIN_DIR=bin\Debug
CALL %~dp0generate-phud.bat
IF ERRORLEVEL 1 ECHO Error creating the phud build configuration for clang+msvc. && EXIT /B 1
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
@ECHO ON