#!/bin/bash
# Copyright (c) 2025 kong9812

# Build with CMake (Debug)
cmake -S core --preset ninja-clang-linux-debug
if [ $? -ne 0 ]; then
    read -p "Error occurred in Debug build. Press Enter to continue..."
fi

# Build with CMake (Release)
cmake -S core --preset ninja-clang-linux-release
if [ $? -ne 0 ]; then
    read -p "Error occurred in Release build. Press Enter to continue..."
fi