@ECHO OFF
REM no SETLOCAL to stay in the build dir so that "test" works
WHERE /q cl.exe || ECHO Could not find the cl.exe program. Check that it is in the path. && EXIT /B 1

setlocal enabledelayedexpansion

REM Fonction pour retirer un exécutable du PATH dans le script
:remove_from_path
set "executable=%~1"
set "current_path=%PATH%"
set "new_path="

for %%d in ("%current_path:;=" "%%d") do (
  set "dir=%%~d"
  set "dir=!dir:~0,1=!" 2>nul || set "dir=%%d"
  if not exist "!dir!\%executable%.exe" (
    if defined new_path (
      set "new_path=!new_path!;!dir!"
    ) else (
      set "new_path=!dir!"
    )
  )
)

if defined new_path (
  setx PATH "!new_path!"
  echo %executable% a été retiré du PATH pour la durée du script.
) else (
  echo Aucun dossier contenant %executable%.exe trouvé dans le PATH.
)
goto :restOfScript

REM si clang ou gcc sont dans le PATH, on les en retire
WHERE clang >nul 2>&1 && call :remove_from_path clang
WHERE gcc >nul 2>&1 && call :remove_from_path gcc

:restOfScript
SET CMAKE_GENERATOR="Visual Studio 18 2026" -A x64 -Thost=x64
SET BUILD_DIR=build
SET BIN_DIR=bin\Debug\Debug
CALL %~dp0generate-phud.bat
IF ERRORLEVEL 1 ECHO Error creating the phud build configuration for Visual Studio. && EXIT /B 1
SET CMAKE_GENERATOR=
SET BUILD_DIR=
SET BIN_DIR=
ENDLOCAL
@ECHO ON