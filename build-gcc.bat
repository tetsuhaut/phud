@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "make" works
where /q mingw32-make || ECHO Could not find the mingw32-make command. Check that it is in the path. && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
REM SET CMAKE_GENERATOR="Eclipse CDT4 - MinGW Makefiles"
SET CMAKE_GENERATOR="Eclipse CDT4 - Ninja"
SET BUILD_DIR=build
SET BIN_DIR=bin\Debug
CALL build.bat
@ECHO OFF
IF ERRORLEVEL 1 ECHO Error building the phud Makefile. && EXIT /B 1
REM ECHO ^color ^&^& ^cls ^&^& ^mingw32-make -j 4 %%* > %BUILD_DIR%\make.bat
REM ECHO ^color ^&^& ^cls ^&^& ^^mingw32-make -j 4 ^&^& test %%* > %BUILD_DIR%\makeTest.bat
ECHO ^color ^&^& ^cls ^&^& ^ninja %%* > %BUILD_DIR%\make.bat
ECHO ^color ^&^& ^cls ^&^& ^ninja ^&^& %BIN_DIR%\unitTests.exe %%* > %BUILD_DIR%\makeTest.bat
PUSHD %BUILD_DIR%
ENDLOCAL
@ECHO ON