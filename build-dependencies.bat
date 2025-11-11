@ECHO OFF

setlocal
IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
SET /P AREYOUSURE=Ce script va recompiler et installer toutes les dependences avec le compilateur %COMPILER%. OK (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END
endlocal

IF NOT DEFINED LIB_DIR ECHO need to define LIB_DIR && EXIT /B 1
IF NOT EXIST %LIB_DIR% ECHO LIB_DIR refers to a non existing directory && EXIT /B 1
IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1

IF EXIST %~dp0\build-dependencies.log (DEL %~dp0\build-dependencies.log)

ECHO Building boost for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building boost for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\boost
IF EXIST build-boost-%COMPILER%.bat (
  build-boost-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-boost-%COMPILER%.bat not found
  ECHO ERROR: build-boost-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building fltk for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building fltk for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\fltk
IF EXIST build-fltk-%COMPILER%.bat (
  build-fltk-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-fltk-%COMPILER%.bat not found
  ECHO ERROR: build-fltk-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building frozen for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building frozen for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\frozen
IF EXIST build-frozen-%COMPILER%.bat (
  build-frozen-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-frozen-%COMPILER%.bat not found
  ECHO ERROR: build-frozen-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building gsl for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building gsl for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\gsl
IF EXIST build-gsl-%COMPILER%.bat (
  build-gsl-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-gsl-%COMPILER%.bat not found
  ECHO ERROR: build-gsl-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building spdlog for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building spdlog for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\spdlog
IF EXIST build-spdlog-%COMPILER%.bat (
  build-spdlog-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-spdlog-%COMPILER%.bat not found
  ECHO ERROR: build-spdlog-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building sqlite for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building sqlite for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\sqlite
IF EXIST build-sqlite-%COMPILER%.bat (
  build-sqlite-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-sqlite-%COMPILER%.bat not found
  ECHO ERROR: build-sqlite-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building stlab for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building stlab for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\stlab
IF EXIST build-stlab-%COMPILER%.bat (
  build-stlab-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-stlab-%COMPILER%.bat not found
  ECHO ERROR: build-stlab-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

ECHO Building utfcpp for %COMPILER%...
ECHO. >> %~dp0\build-dependencies.log
ECHO Building utfcpp for %COMPILER%... >> %~dp0\build-dependencies.log
ECHO. >> %~dp0\build-dependencies.log
PUSHD %LIB_DIR%\utfcpp
IF EXIST build-utfcpp-%COMPILER%.bat (
  build-utfcpp-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
) ELSE (
  ECHO ERROR: build-utfcpp-%COMPILER%.bat not found
  ECHO ERROR: build-utfcpp-%COMPILER%.bat not found >> %~dp0\build-dependencies.log
)
POPD

:END

@ECHO ON
exit