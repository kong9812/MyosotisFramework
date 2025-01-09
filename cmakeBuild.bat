@echo off

:: binフォルダ
if not exist "./bin" (
    mkdir "./bin"
)

:: cmakeでビルド
cmake -S "./core" -B "./bin" -G "Visual Studio 17 2022"

pause