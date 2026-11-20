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
REM hides the "/showIncludes" lines of cl.exe when building with ninja
FOR /F "tokens=2 delims=:" %%a IN ('chcp') DO SET /A OLD_CP=%%a
CHCP 65001 > NUL
CALL %~dp0generate-phud.bat
SET GENERATE_ERROR=%ERRORLEVEL%
CHCP %OLD_CP% > NUL
SET OLD_CP=
IF %GENERATE_ERROR% NEQ 0 ECHO Error creating the phud build configuration for Visual Studio. && EXIT /B 1
SET GENERATE_ERROR=
FOR %%i IN ("%~dp0..") DO SET SRC_DIR=%%~fi

REM build.bat: usual build, without static analysis
SET BUILD_SCRIPT=build-phud-%COMPILER%\build.bat
ECHO @ECHO OFF> %BUILD_SCRIPT%
ECHO SETLOCAL>> %BUILD_SCRIPT%
ECHO REM same code page as CMake: see generate-phud-msvc.bat>> %BUILD_SCRIPT%
ECHO FOR /F "tokens=2 delims=:" %%%%a IN ('chcp') DO SET /A OLD_CP=%%%%a>> %BUILD_SCRIPT%
ECHO CHCP 65001 ^> NUL>> %BUILD_SCRIPT%
ECHO ninja %%*>> %BUILD_SCRIPT%
ECHO SET BUILD_ERROR=%%ERRORLEVEL%%>> %BUILD_SCRIPT%
ECHO CHCP %%OLD_CP%% ^> NUL>> %BUILD_SCRIPT%
ECHO EXIT /B %%BUILD_ERROR%%>> %BUILD_SCRIPT%

REM build-analyze.bat: build with /analyze and the C++ Core Check, in the 'analyze' sub dir
REM so that switching between both builds does not trigger a full rebuild
SET BUILD_SCRIPT=build-phud-%COMPILER%\build-analyze.bat
ECHO @ECHO OFF> %BUILD_SCRIPT%
ECHO SETLOCAL>> %BUILD_SCRIPT%
ECHO REM same code page as CMake: see generate-phud-msvc.bat>> %BUILD_SCRIPT%
ECHO FOR /F "tokens=2 delims=:" %%%%a IN ('chcp') DO SET /A OLD_CP=%%%%a>> %BUILD_SCRIPT%
ECHO CHCP 65001 ^> NUL>> %BUILD_SCRIPT%
ECHO IF NOT EXIST analyze\build.ninja cmake -G %CMAKE_GENERATOR% -S "%SRC_DIR%" -B analyze -DPHUD_MSVC_ANALYZE=ON>> %BUILD_SCRIPT%
ECHO IF ERRORLEVEL 1 GOTO end>> %BUILD_SCRIPT%
ECHO ninja -C analyze %%*>> %BUILD_SCRIPT%
ECHO :end>> %BUILD_SCRIPT%
ECHO SET BUILD_ERROR=%%ERRORLEVEL%%>> %BUILD_SCRIPT%
ECHO CHCP %%OLD_CP%% ^> NUL>> %BUILD_SCRIPT%
ECHO EXIT /B %%BUILD_ERROR%%>> %BUILD_SCRIPT%
SET BUILD_SCRIPT=
SET SRC_DIR=

SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
pushd  build-phud-%COMPILER%
@ECHO ON
