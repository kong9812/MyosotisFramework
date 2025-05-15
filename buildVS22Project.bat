@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: binフォルダ
if not exist "./bin/vs22" (
    mkdir "./bin/vs22"
)

:: cmakeでビルド
cmake -S "./core" -B "./bin/vs22" -G "Visual Studio 17 2022"
if not %ERRORLEVEL%==0 (
    pause
)

endlocal