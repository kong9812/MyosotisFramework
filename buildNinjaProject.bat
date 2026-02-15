@echo off
chcp 65001
setlocal

:: VSのパスとvcvarsallの場所を特定
for /f "delims=" %%i in ('vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set VS_PATH=%%i
set VCVARSALL_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat

echo Using VS: %VS_PATH%

:: コンパイラ環境のロード
call "%VCVARSALL_PATH%" amd64

:: Release構成の生成
echo Configuring Release...
cmake -S core --preset ninja-msvc-release
if %ERRORLEVEL% neq 0 pause

:: Debug構成の生成
echo Configuring Debug...
cmake -S core --preset ninja-msvc-debug
if %ERRORLEVEL% neq 0 pause

endlocal