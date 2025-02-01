@echo off
setlocal enabledelayedexpansion

:: カレントディレクトリを使用
set "SHADER_DIR=%cd%\core\resources\shaders"

:: 出力ディレクトリ（SPIR-Vファイルを保存）
set "OUTPUT_DIR=%SHADER_DIR%\spv"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: .vert と .frag ファイルを全て検索して変換
echo Converting shaders in %SHADER_DIR%...

:: .vert .frag .comp  ファイルの変換
for /r "%SHADER_DIR%" %%f in (*.vert *.frag *.comp) do (
    echo Converting: %%f
    glslangValidator -V "%%f" -o "%OUTPUT_DIR%\%%~nxf.spv"
)

echo Shader conversion completed!
pause