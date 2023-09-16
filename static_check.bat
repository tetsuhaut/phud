@ECHO OFF
where /q cppcheck || ECHO Could not find the cppcheck command. Check that it is in the path. && EXIT /B 1
IF NOT DEFINED VCToolsInstallDir ECHO need to set the VCToolsInstallDir environment variable && EXIT /B 1

cppcheck --check-config -I %Boost_DIR%\include ^
 -I %FLTK_DIR%\install\include -I %frozen_DIR%\include -I %Microsoft.GSL_DIR%\include ^
 -I %spdlog_DIR%\include -I %SQLite3_DIR%\include -I %stlab_DIR%\include ^
 -I %~dp0%src\main\cpp ^
 -I "%VCToolsInstallDir%\include" ^
 -I "%WindowsSdkVerBinPath%\ucrt" ^
 -I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\ucrt" ^
 -I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um" ^
 -I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\shared" ^
 %~dp0%src\main

@ECHO ON

rem cls && cppcheck --project=e:\prog\workspaceCMake\phud\Poker_Head-Up_Display.sln