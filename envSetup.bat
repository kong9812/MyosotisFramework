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

pause