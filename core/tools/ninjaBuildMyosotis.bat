@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

for /f "delims=" %%i in ('vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set VS_PATH=%%i
set VCVARSALL_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat
call "%VCVARSALL_PATH%" amd64

cmake --build ../../bin/ninja_msvc/release --target Myosotis --config Release --parallel
cmake --build ../../bin/ninja_msvc/debug --target Myosotis --config Debug --parallel

endlocal