@echo off
chcp 65001
:: Copyright (c) 2025 kong9812
setlocal

:: Define source and build directories
set QT_SRC_DIR=.qt
set QT_BUILD_DIR=qt_clang

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

    :: Load MSYS2 UCRT64 environment
    echo Setting up MSYS2 UCRT64 environment
    set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"

    :: Run Qt's configure script
    echo Configuring Qt
    if exist "..\%QT_SRC_DIR%\qtbase\.git" (
        call "..\%QT_SRC_DIR%\configure.bat" -submodules qtbase -debug-and-release -- ^
            -DCMAKE_C_COMPILER=clang ^
            -DCMAKE_CXX_COMPILER=clang++ ^
            -DCMAKE_MAKE_PROGRAM=ninja
    ) else (
        call "..\%QT_SRC_DIR%\configure.bat" -init-submodules -submodules qtbase -debug-and-release -- ^
            -DCMAKE_C_COMPILER=clang ^
            -DCMAKE_CXX_COMPILER=clang++ ^
            -DCMAKE_MAKE_PROGRAM=ninja
    )

    :: Build qtbase
    ninja qtbase

    popd
)
endlocal