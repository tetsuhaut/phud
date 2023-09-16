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

rem one letter for the loop variable
FOR %%l IN (boost fltk frozen gsl spdlog sqlite stlab tuplet utfcpp) DO (
  ECHO Building %%l for %COMPILER%...
  ECHO. >> %~dp0\build-dependencies.log
  ECHO Building %%l for %COMPILER%... >> %~dp0\build-dependencies.log
  ECHO. >> %~dp0\build-dependencies.log
  PUSHD %LIB_DIR%\%%l
  build-%%l-%COMPILER%.bat >> %~dp0\build-dependencies.log
  @ECHO OFF
  ECHO Done.
  POPD
)

:END

@ECHO ON
exit