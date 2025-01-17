@echo off

:: git
git --version >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Git is installed.
    git --version
) else (
    echo Start install git.
    :: なければインストール
    winget install --id Git.Git -e --source winget
)

:: cmake
cmake --version >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Cmake is installed.
    cmake --version
) else (
    echo Start install cmake.
    :: なければインストール
    winget install --id Kitware.CMake --source winget
)

:: vswhere
where vswhere >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo vswhere is installed.
) else (
    echo Start install Microsoft.VisualStudio.Installer.
    :: なければインストール
    winget install --id Microsoft.VisualStudio.Installer --exact
)

:: ジャンクション(resources)
mklink /J "bin\Debug\resources" "core\resources"
mklink /J "bin\Release\resources" "core\resources"
mklink /J "bin\FWDLL\resources" "core\resources"
:: ジャンクション(log)
mklink /J "bin\Debug\log" "core\log"
mklink /J "bin\Release\log" "core\log"
mklink /J "bin\FWDLL\log" "core\log"
:: ジャンクション(tools)
mklink /J "bin\Debug\tools" "core\tools"
mklink /J "bin\Release\tools" "core\tools"
mklink /J "bin\FWDLL\tools" "core\tools"
pause