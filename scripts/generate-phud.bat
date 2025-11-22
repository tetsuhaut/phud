IF NOT DEFINED COMPILER ECHO need to set the COMPILER environment variable && EXIT /B 1
IF NOT DEFINED CMAKE_GENERATOR ECHO call build-*.bat instead && EXIT /B 1
IF NOT DEFINED BIN_DIR ECHO call build-*.bat instead && EXIT /B 1
IF NOT DEFINED SCRIPTS_DIR ECHO need to set the SCRIPTS_DIR environment variable && EXIT /B 1

ECHO checking for environment
REM the *_DIR environment variables are hints to help CMake finding the libraries and include dirs
REM one letter for the loop variable
SETLOCAL enabledelayedexpansion
SET "error_found=0"
FOR %%i IN (Boost_DIR FLTK_DIR frozen_DIR Microsoft.GSL_DIR SCRIPTS_DIR SPDLOG_DIR SQLite3_DIR stlab_DIR) DO (
  IF NOT DEFINED %%i ECHO need to set the %%i environment variable && EXIT /B 1
  FOR /F %%j IN ('ECHO %%i') DO (
    IF NOT EXIST !%%j!\NUL ECHO %%i refers to a non existing directory '!%%j!' && SET "error_found=1"
  )
)
IF %error_found% EQU 1 (EXIT /B 1)
ENDLOCAL

IF NOT EXIST %SCRIPTS_DIR%\timecmd.bat ECHO the script 'timecmd.bat' could not be found && EXIT /B 1

SET BUILD_DIR=build-phud-%COMPILER%

IF EXIST %BUILD_DIR% (ECHO deleting build dir '%BUILD_DIR%' && RMDIR /Q /S %BUILD_DIR%)
ECHO creating build dir '%BUILD_DIR%'
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
ECHO calling cmake
cmake -G %CMAKE_GENERATOR% %~dp0\..
POPD
ECHO ^pushd %CD% ^&^& ^format ^&^& ^popd > %BUILD_DIR%\format.bat
ECHO ^color ^&^& ^cls ^&^& ^%BIN_DIR%\unitTests.exe %%* > %BUILD_DIR%\runAllTests.bat
ECHO ^color ^&^& ^cls ^&^& ^%BIN_DIR%\unitTests.exe --run_test=%%* -l all > %BUILD_DIR%\runOneTest.bat
ECHO ^%BIN_DIR%\unitTests.exe --log_level=unit_scope > %BUILD_DIR%\measure.bat
ECHO ^%BIN_DIR%\dbgen.exe -b %BIN_DIR%\tc1591.db -d %CD%\src\test\resources\tc1591 > %BUILD_DIR%\updateDb.bat
ECHO ^%BIN_DIR%\dbgen.exe -b %BIN_DIR%\simpleCGHisto.db -d %CD%\src\test\resources\simpleCGHisto >> %BUILD_DIR%\updateDb.bat
ECHO ^%BIN_DIR%\dbgen.exe -b %BIN_DIR%\simpleTHisto.db -d %CD%\src\test\resources\simpleTHisto >> %BUILD_DIR%\updateDb.bat
ECHO ^%BIN_DIR%\dbgen.exe -b %BIN_DIR%\sabre_laser.db -d %CD%\src\test\resources\sabre_laser >> %BUILD_DIR%\updateDb.bat
ECHO ^del log.txt ^& ^%BIN_DIR%\phud.exe %%* > %BUILD_DIR%\phud.bat
ECHO ^%BIN_DIR%\guiDryRun.exe > %BUILD_DIR%\guiDryRun.bat
ECHO done