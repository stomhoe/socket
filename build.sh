#!/bin/bash

# Exit on error
set -e

# Set build directory
BUILD_DIR="build"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake to configure the project
cmake ..

# Build the project
cmake --build .
