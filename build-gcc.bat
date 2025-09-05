@ECHO OFF



where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1
SET CMAKE_GENERATOR="Ninja"
SET BIN_DIR=bin\Debug
CALL build.bat
IF ERRORLEVEL 1 ECHO Error building the phud build configuration for GCC. && EXIT /B 1
PUSHD %BUILD_DIR%
ECHO ^color ^&^& ^cls ^&^& ^ninja %%* > make.bat
ECHO ^color ^&^& ^cls ^&^& ^ninja ^&^& %BIN_DIR%\unitTests.exe %%* > makeTest.bat
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
@ECHO ON