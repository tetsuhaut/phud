@echo off
setlocal

if "%1"=="" (
    echo Usage: run-tidy.bat fichier.cpp
    exit /b 1
)

REM Désactive tous les checks de style de nommage (readability-identifier-naming)
clang-tidy %1 --checks="-readability-identifier-naming" -- -std=c++23 -Iphud\src\main\cpp -DNOMINMAX -DPHUD_APP_VERSION="0.0.2" -DPHUD_APP_NAME_SHORT="phud" -DPHUD_APP_NAME_LONG="Poker Head-Up Display" -DBOOST_ALL_NO_LIB -DSPDLOG_COMPILED_LIB -DWIN32 -D_WINDOWS -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 -D_ITERATOR_DEBUG_LEVEL=0 -isystem %Boost_DIR%\include -isystem %frozen_DIR%\include -isystem %Microsoft.GSL_DIR%\include -isystem %spdlog_DIR%\include -isystem %EFSW_DIR%\include -isystem %stlab_DIR%\include -isystem %FLTK_DIR% -isystem C:\prog\lib\fltk\fltk-release-1.4.4 -isystem %SQLite3_DIR%\include -fms-extensions -fms-compatibility -fms-compatibility-version=19.44

endlocal
