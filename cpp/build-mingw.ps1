# Windows MinGW Build Script for Pixiv2Billfish

Write-Host "=== Building with MinGW ===" -ForegroundColor Cyan

# Check if MinGW is installed
$mingwPath = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $mingwPath) {
    Write-Host "Error: MinGW not found in PATH!" -ForegroundColor Red
    Write-Host "Please install MinGW-w64 and add it to PATH" -ForegroundColor Yellow
    Write-Host "Download from: https://www.mingw-w64.org/" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found MinGW: $($mingwPath.Source)" -ForegroundColor Green

# Check vcpkg
if (-not (Test-Path "vcpkg")) {
    Write-Host "Cloning vcpkg..." -ForegroundColor Green
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    .\bootstrap-vcpkg.bat
    .\vcpkg integrate install
    cd ..
}

# Install dependencies for MinGW
Write-Host "Installing dependencies..." -ForegroundColor Green
$env:VCPKG_DEFAULT_TRIPLET = "x64-mingw-dynamic"
.\vcpkg\vcpkg install curl:x64-mingw-dynamic sqlite3:x64-mingw-dynamic nlohmann-json:x64-mingw-dynamic spdlog:x64-mingw-dynamic

# Create build directory
if (Test-Path "build-mingw") {
    Remove-Item -Recurse -Force build-mingw
}
New-Item -ItemType Directory -Path build-mingw | Out-Null
cd build-mingw

# Configure CMake with MinGW
Write-Host "Configuring CMake with MinGW Makefiles..." -ForegroundColor Green
$env:VCPKG_DEFAULT_TRIPLET = "x64-mingw-dynamic"
cmake .. -G "MinGW Makefiles" `
    -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_C_COMPILER=gcc `
    -DCMAKE_CXX_COMPILER=g++

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    cd ..
    exit 1
}

# Build
Write-Host "Building with MinGW..." -ForegroundColor Green
mingw32-make -j$env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    cd ..
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable location: build-mingw\Pixiv2Billfish.exe" -ForegroundColor Cyan

cd ..
