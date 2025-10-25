# Quick MinGW Build Script (if dependencies already installed)

Write-Host "=== Quick MinGW Build ===" -ForegroundColor Cyan

# Create build directory
if (Test-Path "build-mingw") {
    Remove-Item -Recurse -Force build-mingw
}
New-Item -ItemType Directory -Path build-mingw | Out-Null
cd build-mingw

# Configure CMake with MinGW
Write-Host "Configuring CMake..." -ForegroundColor Green
cmake .. -G "MinGW Makefiles" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_C_COMPILER=gcc `
    -DCMAKE_CXX_COMPILER=g++

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Write-Host "Make sure you have installed the dependencies:" -ForegroundColor Yellow
    Write-Host "  - libcurl-devel" -ForegroundColor Yellow
    Write-Host "  - sqlite3-devel" -ForegroundColor Yellow
    Write-Host "  - nlohmann-json" -ForegroundColor Yellow
    Write-Host "  - spdlog" -ForegroundColor Yellow
    cd ..
    exit 1
}

# Build
Write-Host "Building..." -ForegroundColor Green
mingw32-make -j$env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "Executable location: build-mingw\Pixiv2Billfish.exe" -ForegroundColor Cyan
} else {
    Write-Host "Build failed!" -ForegroundColor Red
}

cd ..
