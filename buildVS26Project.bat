@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: Create bin folder for VS2022 if it doesn't exist
if not exist "./bin/vs26" (
    mkdir "./bin/vs26"
)

:: Build with CMake
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -S "./core" -B "./bin/vs26" -G "Visual Studio 18 2026"
if not %ERRORLEVEL%==0 (
    pause
)

endlocal