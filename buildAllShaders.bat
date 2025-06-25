@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: Use current directory
set "SHADER_DIR=%cd%\core\resources\shaders"

:: Output directory for SPIR-V files
set "OUTPUT_DIR=%SHADER_DIR%\spv"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Convert all .vert, .frag, and .comp shader files
echo Converting shaders in %SHADER_DIR%...

for /r "%SHADER_DIR%" %%f in (*.vert *.frag *.comp) do (
    echo Converting: %%f
    glslangValidator -V "%%f" -o "%OUTPUT_DIR%\%%~nxf.spv"
)

echo Shader conversion completed!
if not defined SKIP_PAUSE pause 
endlocal