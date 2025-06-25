@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: Create bin folder for VS2022 if it doesn't exist
if not exist "./bin/vs22" (
    mkdir "./bin/vs22"
)

:: Build with CMake
cmake -S "./core" -B "./bin/vs22" -G "Visual Studio 17 2022"
if not %ERRORLEVEL%==0 (
    pause
)

endlocal