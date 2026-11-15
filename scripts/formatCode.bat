REM clang-format is provided by installing llvm with scoop
PUSHD ..
for /r src %%f in (*.cpp *.hpp) do clang-format -i "%%f"
POPD