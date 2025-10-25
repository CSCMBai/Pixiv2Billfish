# Simple MinGW Build Script (Using MSYS2 packages)

Write-Host "=== Building with MinGW (MSYS2) ===" -ForegroundColor Cyan

# Check if we're in MSYS2 environment
if ($env:MSYSTEM) {
    Write-Host "Detected MSYS2 environment: $env:MSYSTEM" -ForegroundColor Green
} else {
    Write-Host "Warning: Not in MSYS2 environment" -ForegroundColor Yellow
    Write-Host "Recommended: Use MSYS2 MinGW64 shell" -ForegroundColor Yellow
    Write-Host "" 
    Write-Host "Or install dependencies manually:" -ForegroundColor Yellow
    Write-Host "  pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake" -ForegroundColor Cyan
    Write-Host "  pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-sqlite3" -ForegroundColor Cyan
    Write-Host "  pacman -S mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-spdlog" -ForegroundColor Cyan
    Write-Host ""
}

# Create build directory
if (Test-Path "build-mingw") {
    Write-Host "Removing old build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build-mingw
}
New-Item -ItemType Directory -Path build-mingw | Out-Null
cd build-mingw

# Configure CMake with MinGW
Write-Host "Configuring CMake with MinGW Makefiles..." -ForegroundColor Green
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "" -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Red
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install dependencies using MSYS2:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "1. Install MSYS2 from: https://www.msys2.org/" -ForegroundColor Cyan
    Write-Host "2. Open 'MSYS2 MinGW 64-bit' terminal" -ForegroundColor Cyan
    Write-Host "3. Run these commands:" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "   pacman -Syu" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-gcc" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-cmake" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-make" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-curl" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-sqlite3" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-nlohmann-json" -ForegroundColor Green
    Write-Host "   pacman -S mingw-w64-x86_64-spdlog" -ForegroundColor Green
    Write-Host ""
    Write-Host "4. Add to PATH: C:\msys64\mingw64\bin" -ForegroundColor Cyan
    Write-Host ""
    cd ..
    exit 1
}

# Build
Write-Host "Building with MinGW..." -ForegroundColor Green
$numProcs = $env:NUMBER_OF_PROCESSORS
if ($numProcs) {
    mingw32-make -j $numProcs
} else {
    mingw32-make -j 8
}

if ($LASTEXITCODE -eq 0) {
    Write-Host "" -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Executable: build-mingw\Pixiv2Billfish.exe" -ForegroundColor Cyan
    Write-Host ""
} else {
    Write-Host "" -ForegroundColor Red
    Write-Host "Build failed!" -ForegroundColor Red
}

cd ..
