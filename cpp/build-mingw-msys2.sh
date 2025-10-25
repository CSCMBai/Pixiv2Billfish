#!/bin/bash
# MSYS2 MinGW Build Script

echo "=== Building with MinGW (MSYS2) ==="

# Check if in MSYS2 environment
if [ -z "$MSYSTEM" ]; then
    echo "Error: Not in MSYS2 environment"
    echo "Please run this from MSYS2 MinGW 64-bit shell"
    exit 1
fi

echo "MSYS2 Environment: $MSYSTEM"

# Check dependencies
echo "Checking dependencies..."
command -v g++ >/dev/null 2>&1 || { echo "g++ not found! Install: pacman -S mingw-w64-x86_64-gcc"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo "cmake not found! Install: pacman -S mingw-w64-x86_64-cmake"; exit 1; }

# Check libraries
echo "Checking libraries..."
MISSING_LIBS=0

if ! pacman -Q mingw-w64-x86_64-curl >/dev/null 2>&1; then
    echo "Missing: mingw-w64-x86_64-curl"
    MISSING_LIBS=1
fi

if ! pacman -Q mingw-w64-x86_64-sqlite3 >/dev/null 2>&1; then
    echo "Missing: mingw-w64-x86_64-sqlite3"
    MISSING_LIBS=1
fi

if ! pacman -Q mingw-w64-x86_64-nlohmann-json >/dev/null 2>&1; then
    echo "Missing: mingw-w64-x86_64-nlohmann-json"
    MISSING_LIBS=1
fi

if ! pacman -Q mingw-w64-x86_64-spdlog >/dev/null 2>&1; then
    echo "Missing: mingw-w64-x86_64-spdlog"
    MISSING_LIBS=1
fi

if [ $MISSING_LIBS -eq 1 ]; then
    echo ""
    echo "Install missing packages with:"
    echo "  pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-sqlite3 \\"
    echo "            mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-spdlog"
    exit 1
fi

echo "All dependencies found!"

# Create build directory
rm -rf build-mingw
mkdir build-mingw
cd build-mingw

# Configure
echo "Configuring CMake..."
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    cd ..
    exit 1
fi

# Build
echo "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "============================================"
    echo "Build completed successfully!"
    echo "============================================"
    echo ""
    echo "Executable: build-mingw/Pixiv2Billfish.exe"
    echo ""
else
    echo "Build failed!"
    cd ..
    exit 1
fi

cd ..
