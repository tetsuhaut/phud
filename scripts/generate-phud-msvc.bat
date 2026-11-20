@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "test" works
WHERE /q cl.exe || ECHO Could not find the devenv.exe program. Check that it is in the path. && EXIT /B 1
WHERE /q ninja.exe || ECHO Could not find the devenv.exe program. Check that it is in the path. && EXIT /B 1
WHERE /q clang && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE /q gcc && (echo ERROR: gcc is in the PATH! && EXIT /B 1)

SET CMAKE_GENERATOR=Ninja
SET BUILD_DIR=build
SET BIN_DIR=bin\Debug
REM avoid ninja to display a message at each include
SET VSLANG=1033
CALL %~dp0generate-phud.bat
IF ERRORLEVEL 1 ECHO Error creating the phud build configuration for Visual Studio. && EXIT /B 1
REM build.bat: usual build, without static analysis
ECHO ^ninja %%* > build-phud-%COMPILER%\build.bat
REM build-analyze.bat: build with /analyze and the C++ Core Check, in the 'analyze' sub dir
REM so that switching between both builds does not trigger a full rebuild
ECHO ^@IF NOT EXIST analyze\build.ninja ^cmake -G %CMAKE_GENERATOR% -S "%~dp0.." -B analyze -DPHUD_MSVC_ANALYZE=ON ^|^| ^EXIT /B 1 > build-phud-%COMPILER%\build-analyze.bat
ECHO ^ninja -C analyze %%* >> build-phud-%COMPILER%\build-analyze.bat
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
pushd  build-phud-%COMPILER%
@ECHO ON