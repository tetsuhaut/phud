rem @ECHO OFF
REM got from scoop (installed with llvm)
IF NOT DEFINED COMPILER ECHO need to define COMPILER && EXIT /B 1
where /q clang-tidy || ECHO Could not find the clang-tidy command. Make sure it is in the path. && EXIT /B 1

REM Vérifier si compile_commands.json existe dans le répertoire build
SET BUILD_DIR=build-phud-%COMPILER%
IF NOT EXIST "%BUILD_DIR%\compile_commands.json" (
  ECHO Could not find the file "%BUILD_DIR%\compile_commands.json"
  EXIT /B 1
)

cls

REM Exécuter clang-tidy avec la base de données de compilation
REM --config-file force l'utilisation du .clang-tidy à la racine du projet
clang-tidy -p "%BUILD_DIR%" --config-file=".clang-tidy" --extra-arg=-std=c++23 %*
@ECHO ON