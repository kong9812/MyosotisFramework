@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

if not exist "./bin/ninja_clang/debug" (
    mkdir "./bin/ninja_clang/debug"
)

:: Build with CMake
cmake -S core --preset ninja-clang-debug
if not %ERRORLEVEL%==0 (
    pause
)

:: todo. wip build
::cmake --build ./bin/mingw_gcc/debug --target MyosotisEngine --config Debug --parallel

endlocal