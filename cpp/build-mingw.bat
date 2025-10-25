@echo off
REM MinGW Build Script for Windows Command Prompt

echo === Building with MinGW ===

REM Check MinGW
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found in PATH!
    echo Please install MinGW and add to PATH
    exit /b 1
)

echo Found MinGW

REM Create build directory
if exist build-mingw rmdir /s /q build-mingw
mkdir build-mingw
cd build-mingw

REM Configure
echo Configuring CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ============================================
    echo CMake configuration failed!
    echo ============================================
    echo.
    echo Please install dependencies using MSYS2:
    echo.
    echo 1. Install MSYS2: https://www.msys2.org/
    echo 2. Open MSYS2 MinGW 64-bit terminal
    echo 3. Run: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
    echo         mingw-w64-x86_64-curl mingw-w64-x86_64-sqlite3
    echo         mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-spdlog
    echo 4. Add to PATH: C:\msys64\mingw64\bin
    echo.
    cd ..
    exit /b 1
)

REM Build
echo Building...
mingw32-make -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo Build completed successfully!
    echo ============================================
    echo.
    echo Executable: build-mingw\Pixiv2Billfish.exe
    echo.
) else (
    echo.
    echo Build failed!
)

cd ..
