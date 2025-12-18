REM @ECHO OFF
REM got from https://www.boost.org/users/download/
SETLOCAL
REM as boost doesn't know visual studio 2026 yet
REM /MTd : multithread, static runtime, debug
REM /Zi : add debug symbols (pdb)
rem SET TOOLSET=msvc cxxflags="/std:c++20 /MTd /Zi" linkflags="/MACHINE:X64"

WHERE /q clang && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE /q gcc && (echo ERROR: gcc is in the PATH! && EXIT /B 1)

SET TOOLSET=msvc
SET MAKE_EXECUTABLE=ninja
CALL %~dp0build-boost.bat

ENDLOCAL
@ECHO ON