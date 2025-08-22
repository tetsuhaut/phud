@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "ninja" works
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
SET CMAKE_GENERATOR="Ninja"
SET BUILD_DIR=build
SET BIN_DIR=bin\Debug
CALL build.bat
IF ERRORLEVEL 1 ECHO Error building the phud Makefile. && EXIT /B 1
ECHO ^color ^&^& ^cls ^&^& ^ninja %%* > %BUILD_DIR%\make.bat
ECHO ^color ^&^& ^cls ^&^& ^ninja ^&^& %BIN_DIR%\unitTests.exe %%* > %BUILD_DIR%\makeTest.bat
PUSHD %BUILD_DIR%
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
@ECHO ON