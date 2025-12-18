@ECHO OFF
REM got from https://www.boost.org/users/download/
REM to build b2 without dependencies : 
REM cd boost_1_89_0\tools\build\src\engine
REM g++ -O2 -static -static-libgcc -static-libstdc++ *.cpp -o b2.exe
REM copy b2.exe ..\..\..\..\b2.exe
REM -static : Link statique complet
REM -static-libgcc : Runtime GCC statique
REM -static-libstdc++ : Librairie C++ statique
REM avoid mixing compilers
WHERE clang >nul 2>&1 && (echo ERROR: clang is in the PATH! && EXIT /B 1)
WHERE cl >nul 2>&1 && (echo ERROR: cl is in the PATH! && EXIT /B 1)

SETLOCAL
SET TOOLSET=gcc
SET MAKE_EXECUTABLE=ninja
CALL %~dp0build-boost.bat

ENDLOCAL
@ECHO ON