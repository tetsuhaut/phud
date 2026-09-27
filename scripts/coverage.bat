@echo off
where /q OpenCppCoverage.exe || ECHO Could not find the OpenCppCoverage.exe command. Check that it is in the path. && EXIT /B 1

SET BUILD_DIR=coverage
IF EXIST %BUILD_DIR% (ECHO deleting build dir '%BUILD_DIR%' && RMDIR /Q /S %BUILD_DIR%)
ECHO creating build dir '%BUILD_DIR%'
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%

ECHO building
cmake -B build ..
cmake --build build --config Debug ..

ECHO run OpenCppCoverage
OpenCppCoverage.exe --sources=src\main\cpp --excluded_sources=src\test\cpp -- build\bin\Debug\Debug\unitTests.exe

POPD