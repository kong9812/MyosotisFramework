#!/bin/bash
# Copyright (c) 2025 kong9812

# Define source and build directories
QT_SRC_DIR=".qt"
QT_BUILD_DIR="qt_clang"

# Check if Qt is already built
if [ ! -d "$QT_BUILD_DIR" ]; then

    # Clone Qt source if not already present
    if [ ! -d "$QT_SRC_DIR" ]; then
        mkdir "$QT_SRC_DIR"

        pushd "$QT_SRC_DIR"
        git clone --branch v6.9.0 https://code.qt.io/qt/qt5.git .
        popd
    fi

    # Create build directory
    echo "Creating build directory: $QT_BUILD_DIR"
    mkdir "$QT_BUILD_DIR"
    pushd "$QT_BUILD_DIR"

    # Run Qt's configure script
    echo "Configuring Qt"
    if [ -d "../$QT_SRC_DIR/qtbase/.git" ]; then
        ../$QT_SRC_DIR/configure -submodules qtbase -debug-and-release -- \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_MAKE_PROGRAM=ninja
    else
        ../$QT_SRC_DIR/configure -init-submodules -submodules qtbase -debug-and-release -- \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_MAKE_PROGRAM=ninja
    fi

    # Build qtbase
    ninja qtbase

    popd
fi