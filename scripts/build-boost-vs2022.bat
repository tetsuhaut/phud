@ECHO OFF
REM got from https://www.boost.org/users/download/
SETLOCAL
SET TOOLSET=msvc
SET MAKE_EXECUTABLE=ninja
CALL build-boost.bat

ENDLOCAL
@ECHO ON