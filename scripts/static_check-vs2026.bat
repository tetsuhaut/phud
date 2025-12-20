@ECHO OFF
where /q cppcheck || ECHO Could not find the cppcheck command. Check that it is in the path. && EXIT /B 1
IF NOT DEFINED VCToolsInstallDir ECHO need to set the VCToolsInstallDir environment variable && EXIT /B 1
IF NOT DEFINED COMPILER ECHO need to set the COMPILER environment variable && EXIT /B 1

cppcheck ^
 --check-config ^
 -I %Boost_DIR%\..\..\..\include ^
 -I %FLTK_DIR%\fltk-build-%COMPILER%\install\include ^
 -I %frozen_DIR%\include ^
 -I %Microsoft.GSL_DIR%\include ^
 -I %spdlog_DIR%\include ^
 -I %SQLite3_DIR%\..\include ^
 -I %stlab_DIR%\include ^
 -I %~dp0%src\main\cpp ^
 -I "%VCToolsInstallDir%\include" ^
 -I "%WindowsSdkVerBinPath%\x64\ucrt" ^
 -I "%WindowsSdkDir%Include\%WindowsSDKLibVersion%\ucrt" ^
 -I "%WindowsSdkDir%Include\%WindowsSDKLibVersion%\um" ^
 -I "%WindowsSdkDir%Include\%WindowsSDKLibVersion%\shared" ^
 src\main

@ECHO ON

rem cls && cppcheck --project=e:\prog\workspaceCMake\phud\Poker_Head-Up_Display.sln