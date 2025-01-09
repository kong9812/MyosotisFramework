@echo off

:: git
git --version >nul 2>nul
:: gitの存在チェック
if %ERRORLEVEL% EQU 0 (
    echo Git is installed.
    git --version
) 
else (
    echo Git is not installed.
    :: なければインストール
    winget install --id Git.Git -e --source winget
)

:: binフォルダ
if not exist "./bin" (
    mkdir "./bin"
)

:: cmakeでビルド
cmake -S "./core" -B "./bin" -G "Visual Studio 17 2022"

pause