@ECHO OFF
REM got from https://www.boost.org/users/download/
SETLOCAL
SET TOOLSET=clang-win
SET MAKE_EXECUTABLE=ninja
CALL %~dp0build-boost.bat
ENDLOCAL
@ECHO ON