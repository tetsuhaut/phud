@ECHO OFF
where /q gcc || ECHO Could not find the gcc command. Check that it is in the path. && EXIT /B 1
where /q cppcheck || ECHO Could not find the cppcheck command. Check that it is in the path. && EXIT /B 1

REM Trouver le répertoire de GCC
for /f %%i in ('where gcc 2^>nul') do set GCC_DIR=%%~pi

REM Déduire le répertoire racine de GCC
set GCC_ROOT=%GCC_DIR%..\
pushd %GCC_ROOT%
set GCC_ROOT=%CD%
popd
REM Déduire le répertoire C++ de GCC
for /f %%i in ('gcc -dumpversion') do set GCC_VERSION=%%i

cppcheck ^
 --check-config ^
 -I %Boost_DIR%\include ^
 -I %FLTK_DIR%\install\include ^
 -I %frozen_DIR%\include ^
 -I %Microsoft.GSL_DIR%\include ^
 -I %spdlog_DIR%\include ^
 -I %SQLite3_DIR%\include ^
 -I %stlab_DIR%\include ^
 -I %~dp0%src\main\cpp ^
 -I "%GCC_ROOT%\include" ^
 -I "%GCC_ROOT%\include\c++\%GCC_VERSION%" ^
 %~dp0%src\main\cpp

@ECHO ON

rem cls && cppcheck --project=e:\prog\workspaceCMake\phud\Poker_Head-Up_Display.sln