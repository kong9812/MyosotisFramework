@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: git
where git >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Git is installed.
) else (
    echo Start install git.
    :: go install
    winget install --id Git.Git -e --source winget
)

:: cmake
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Cmake is installed.
) else (
    echo Start install cmake.
    :: go install
    winget install --id Kitware.CMake --source winget
)

:: vswhere
where vswhere >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Microsoft.VisualStudio.Locator is installed.
) else (
    echo Start install Microsoft.VisualStudio.Locator.
    :: go install
    winget install --id Microsoft.VisualStudio.Locator -e --source winget
)

:: Ninja
where Ninja >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Ninja is installed.
) else (
    echo Start install Ninja.
    :: go install
    winget install --id Ninja-build.Ninja -e --source winget
)

:: shaders
set SKIP_PAUSE=1
call "buildAllShaders.bat"

:: QT
call "setupQT_MSVC.bat"

:: build ninja project
call "buildNinjaProject.bat"

:: test build (ninja: build Myosotis release)
pushd ".\core"
call "tools\ninjaBuildMyosotis_release.bat"
popd

:: build vs22 project
call "buildVS22Project.bat"

:: test build (ninja: build Myosotis release)
pushd ".\core"
call "tools\vs22BuildMyosotis_release.bat"
popd

echo welcome!!
if not defined SKIP_PAUSE pause 
endlocal