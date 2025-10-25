# Windows Build Script for Pixiv2Billfish

# Check vcpkg
if (-not (Test-Path "vcpkg")) {
    Write-Host "Cloning vcpkg..." -ForegroundColor Green
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    .\bootstrap-vcpkg.bat
    .\vcpkg integrate install
    cd ..
}

# Install dependencies
Write-Host "Installing dependencies..." -ForegroundColor Green
.\vcpkg\vcpkg install curl:x64-windows sqlite3:x64-windows nlohmann-json:x64-windows spdlog:x64-windows

# Create build directory
if (Test-Path "build") {
    Remove-Item -Recurse -Force build
}
New-Item -ItemType Directory -Path build | Out-Null
cd build

# Configure CMake
Write-Host "Configuring CMake..." -ForegroundColor Green
cmake .. -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

# Build
Write-Host "Building..." -ForegroundColor Green
cmake --build . --config Release -j

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable location: build\Release\Pixiv2Billfish.exe" -ForegroundColor Cyan

cd ..
