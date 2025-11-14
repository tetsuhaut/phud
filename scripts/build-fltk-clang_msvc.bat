@ECHO OFF
REM got from https://www.fltk.org/ or https://github.com/fltk/fltk
SETLOCAL
IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q 7z || ECHO Could not find the 7z command. Check that it is in the path. && EXIT /B 1
where /q cmake || ECHO Could not find the cmake command. Check that it is in the path. && EXIT /B 1
where /q ninja || ECHO Could not find the ninja command. Check that it is in the path. && EXIT /B 1SET LIB_NAME=fltk
SET VERSION=1.4.4
REM SET VERSION=master
SET LIB_NAME=fltk
SET SRC_DIR=%~dp0%LIB_NAME%-%VERSION%
SET ZIP_NAME_FROM_FLTK_WEB_SITE=%~dp0%LIB_NAME%-%VERSION%-source
REM SET ZIP_NAME_FROM_FLTK_WEB_SITE=%~dp0%LIB_NAME%-%VERSION%
IF EXIST %SRC_DIR% GOTO :build
REM look for fltk-a.b.c-source.zip
IF EXIST %ZIP_NAME_FROM_FLTK_WEB_SITE%.zip (
  7z x %ZIP_NAME_FROM_FLTK_WEB_SITE%.zip
  GOTO :build
) ELSE (
  ECHO Could not find the archive file %ZIP_NAME_FROM_FLTK_WEB_SITE%.zip
)
REM look for fltk-a.b.c-source.tar.bz2
IF EXIST %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar.bz2 (
  7z x %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar.bz2
  7z x %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar
  DEL %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar
  GOTO :build
) ELSE (
  ECHO Could not find the archive file %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar.bz2
)
REM look for fltk-a.b.c-source.tar.gz
IF EXIST %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar.gz (
  7z x %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar.gz
  7z x %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar
  DEL %ZIP_NAME_FROM_FLTK_GITHUB%.tar
  GOTO :build
) ELSE (
  ECHO Could not find the archive file %ZIP_NAME_FROM_FLTK_WEB_SITE%.tar.gz
)
REM look for release-a.b.c.zip
SET ZIP_NAME_FROM_FLTK_GITHUB=%~dp0release-%VERSION%
IF EXIST %ZIP_NAME_FROM_FLTK_GITHUB%.zip (
  7z x %ZIP_NAME_FROM_FLTK_GITHUB%.tar.zip
  GOTO :build
) ELSE (
  ECHO Could not find the archive file %ZIP_NAME_FROM_FLTK_GITHUB%.zip
)
REM look for release-a.b.c.tar.gz
IF EXIST %ZIP_NAME_FROM_FLTK_GITHUB%.tar.gz (
  7z x %ZIP_NAME_FROM_FLTK_GITHUB%.tar.gz
  7z x %ZIP_NAME_FROM_FLTK_GITHUB%.tar
  DEL %ZIP_NAME_FROM_FLTK_GITHUB%.tar
  GOTO :build
) ELSE (
  ECHO Could not find the archive file %ZIP_NAME_FROM_FLTK_GITHUB%.tar.gz
)
REM look for fltk-release-a.b.c.zip
SET ZIP_NAME_FROM_FLTK_GITHUB=%~dp0%LIB_NAME%-release-%VERSION%
IF EXIST %ZIP_NAME_FROM_FLTK_GITHUB%.zip (
  7z x %ZIP_NAME_FROM_FLTK_GITHUB%.zip
  SET SRC_DIR=%~dp0%LIB_NAME%-release-%VERSION%
  GOTO :build
) ELSE (
  ECHO Could not find the archive file %ZIP_NAME_FROM_FLTK_GITHUB%.tar.gz
)
EXIT /B 1

:build
SET BUILD_DIR=%~dp0\%LIB_NAME%-build-%COMPILER%
IF EXIST %BUILD_DIR% (RMDIR /Q /S %BUILD_DIR%)
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%

cmake ^
  -G "Ninja" ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -D CMAKE_BUILD_TYPE=Debug ^
  -D CMAKE_INSTALL_PREFIX=%BUILD_DIR%/install ^
  -D FLTK_BUILD_EXAMPLES=ON ^
  -D FLTK_BUILD_TEST=OFF ^
  -D FLTK_MSVC_RUNTIME_DLL=OFF ^
%SRC_DIR%
ninja all
ninja install
POPD
ENDLOCAL