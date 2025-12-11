@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

for /f "delims=" %%i in ('vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set VS_PATH=%%i
for /f "delims=" %%j in ('dir /b /ad "%VS_PATH%\VC\Tools\MSVC" ^| sort /r') do set MSVC_VERSION=%%j
set MSVC_VERSION=%MSVC_VERSION: =%

set CL_PATH=%VS_PATH%\VC\Tools\MSVC\%MSVC_VERSION%\bin\Hostx64\x64\cl.exe
set VCVARSALL_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat

echo vs: %VS_PATH%
echo cl: %CL_PATH%
echo vcvarsall: %VCVARSALL_PATH%

call "%VCVARSALL_PATH%" amd64
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="%CL_PATH%" -DCMAKE_CXX_COMPILER="%CL_PATH%" -S .\core -B .\bin\ninja
endlocal