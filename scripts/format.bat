@ECHO OFF
REM got from scoop (installed with llvm)
where /q clang-format || ECHO Could not find the clang-format command. Make sure it is in the path. && EXIT /B 1
for /r src %%f in (*.cpp *.hpp) do clang-format -i "%%f"
@ECHO ON