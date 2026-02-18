@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: Define source and build directories
set QT_SRC_DIR=.qt
set QT_BUILD_DIR=qt_msvc

:: Check if Qt is already built
if not exist "%QT_BUILD_DIR%" (

    :: Clone Qt source if not already present
    if not exist "%QT_SRC_DIR%" (
        mkdir "%QT_SRC_DIR%"
        :: Hide the folder
        attrib +h "%QT_SRC_DIR%"

        pushd %QT_SRC_DIR%
        git clone --branch v6.9.0 https://code.qt.io/qt/qt5.git .
        popd
    )

    :: Create build directory
    echo Creating build directory: %QT_BUILD_DIR%
    mkdir "%QT_BUILD_DIR%"
    pushd "%QT_BUILD_DIR%"

    :: Load Visual Studio environment for 64-bit builds
    echo Setting up Visual Studio environment
    for /f "delims=" %%i in ('vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do (
        call "%%i\VC\Auxiliary\Build\vcvarsall.bat" amd64
    )

    :: Run Qt's configure script with only qtbase module
    echo Configuring Qt
    if exist "..\%QT_SRC_DIR%\qtbase\.git" (
        call "..\%QT_SRC_DIR%\configure.bat" -submodules qtbase -debug-and-release
    ) else (
        call "..\%QT_SRC_DIR%\configure.bat" -init-submodules -submodules qtbase -debug-and-release
    )
    :: Build qtbase
    ninja qtbase

    popd
)
endlocal