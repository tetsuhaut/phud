@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "test" works
where /q cl.exe || ECHO Could not find the cl.exe program. Check that it is in the path. && EXIT /B 1
where /q link.exe || ECHO Could not find the link.exe program. Check that it is in the path. && EXIT /B 1
where /q clang.exe || ECHO Could not find the clang.exe program. Check that it is in the path. && EXIT /B 1
where /q ninja.exe || ECHO Could not find the ninja.exe program. Check that it is in the path. && EXIT /B 1
SET CMAKE_GENERATOR="Ninja" -DCMAKE_CXX_COMPILER=clang-cl
SET BIN_DIR=bin\Debug\Debug
CALL %~dp0generate-phud.bat
IF ERRORLEVEL 1 ECHO Error creating the phud build configuration for clang + msvc. && EXIT /B 1
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
pushd build-phud-%COMPILER%
ECHO ^cls ^&^& ^ninja ^clean ^&^& ^ninja > %BUILD_DIR%\build.bat
@ECHO ON