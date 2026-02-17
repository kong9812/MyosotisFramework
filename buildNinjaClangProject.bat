@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: Build with CMake
cmake -S core --preset ninja-clang-debug
if not %ERRORLEVEL%==0 (
    pause
)
cmake -S core --preset ninja-clang-release
if not %ERRORLEVEL%==0 (
    pause
)

endlocal