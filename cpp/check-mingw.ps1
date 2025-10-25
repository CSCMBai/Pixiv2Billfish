# Check MSYS2 MinGW Environment

Write-Host "=== Checking MSYS2 MinGW Environment ===" -ForegroundColor Cyan
Write-Host ""

$allGood = $true

# Check compiler
Write-Host "Checking g++..." -NoNewline
$gcc = Get-Command g++ -ErrorAction SilentlyContinue
if ($gcc) {
    Write-Host " OK" -ForegroundColor Green
    Write-Host "  Location: $($gcc.Source)" -ForegroundColor Gray
    $version = & g++ --version 2>&1 | Select-Object -First 1
    Write-Host "  Version: $version" -ForegroundColor Gray
} else {
    Write-Host " NOT FOUND" -ForegroundColor Red
    $allGood = $false
}

# Check cmake
Write-Host "Checking cmake..." -NoNewline
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    Write-Host " OK" -ForegroundColor Green
    Write-Host "  Location: $($cmake.Source)" -ForegroundColor Gray
} else {
    Write-Host " NOT FOUND" -ForegroundColor Red
    $allGood = $false
}

# Check make
Write-Host "Checking make..." -NoNewline
$make = Get-Command mingw32-make -ErrorAction SilentlyContinue
if ($make) {
    Write-Host " OK" -ForegroundColor Green
    Write-Host "  Location: $($make.Source)" -ForegroundColor Gray
} else {
    Write-Host " NOT FOUND" -ForegroundColor Red
    $allGood = $false
}

Write-Host ""
Write-Host "=== Checking Libraries ===" -ForegroundColor Cyan
Write-Host ""

# Common MSYS2 paths
$msys2Paths = @(
    "C:\msys64\mingw64",
    "D:\msys64\mingw64",
    "C:\msys2\mingw64",
    "D:\msys2\mingw64"
)

$msys2Root = $null
foreach ($path in $msys2Paths) {
    if (Test-Path $path) {
        $msys2Root = $path
        Write-Host "Found MSYS2 MinGW64: $msys2Root" -ForegroundColor Green
        break
    }
}

if (-not $msys2Root) {
    Write-Host "MSYS2 MinGW64 directory not found!" -ForegroundColor Red
    Write-Host "Please install MSYS2 from: https://www.msys2.org/" -ForegroundColor Yellow
    $allGood = $false
} else {
    # Check libraries
    $libPath = Join-Path $msys2Root "lib"
    $includePath = Join-Path $msys2Root "include"
    
    $libraries = @{
        "libcurl" = @("libcurl.a", "libcurl.dll.a")
        "sqlite3" = @("libsqlite3.a", "libsqlite3.dll.a")
        "spdlog" = @("libspdlog.a", "libspdlog.dll.a")
    }
    
    $headers = @{
        "curl" = "curl\curl.h"
        "sqlite3" = "sqlite3.h"
        "spdlog" = "spdlog\spdlog.h"
        "nlohmann-json" = "nlohmann\json.hpp"
    }
    
    foreach ($lib in $libraries.Keys) {
        Write-Host "Checking $lib..." -NoNewline
        $found = $false
        foreach ($file in $libraries[$lib]) {
            $fullPath = Join-Path $libPath $file
            if (Test-Path $fullPath) {
                $found = $true
                break
            }
        }
        if ($found) {
            Write-Host " OK" -ForegroundColor Green
        } else {
            Write-Host " NOT FOUND" -ForegroundColor Red
            Write-Host "  Install: pacman -S mingw-w64-x86_64-$lib" -ForegroundColor Yellow
            $allGood = $false
        }
    }
    
    foreach ($header in $headers.Keys) {
        Write-Host "Checking $header headers..." -NoNewline
        $headerFile = Join-Path $includePath $headers[$header]
        if (Test-Path $headerFile) {
            Write-Host " OK" -ForegroundColor Green
        } else {
            Write-Host " NOT FOUND" -ForegroundColor Red
            $pkgName = if ($header -eq "nlohmann-json") { "nlohmann-json" } else { $header }
            Write-Host "  Install: pacman -S mingw-w64-x86_64-$pkgName" -ForegroundColor Yellow
            $allGood = $false
        }
    }
}

Write-Host ""
Write-Host "=== Environment Variables ===" -ForegroundColor Cyan
Write-Host ""

# Check PATH
Write-Host "PATH contains:" -ForegroundColor Gray
$pathEntries = $env:PATH -split ';'
$mingwInPath = $false
foreach ($entry in $pathEntries) {
    if ($entry -like "*mingw64\bin*") {
        Write-Host "  $entry" -ForegroundColor Green
        $mingwInPath = $true
    }
}

if (-not $mingwInPath) {
    Write-Host "  MinGW64 not in PATH!" -ForegroundColor Red
    if ($msys2Root) {
        $binPath = Join-Path $msys2Root "bin"
        Write-Host "  Add to PATH: $binPath" -ForegroundColor Yellow
    }
    $allGood = $false
}

Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host ""

if ($allGood) {
    Write-Host "All checks passed! You can build the project." -ForegroundColor Green
    Write-Host "Run: .\build-mingw-simple.ps1" -ForegroundColor Cyan
} else {
    Write-Host "Some dependencies are missing!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Quick fix (run in MSYS2 MinGW 64-bit terminal):" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make \\" -ForegroundColor Cyan
    Write-Host "          mingw-w64-x86_64-curl mingw-w64-x86_64-sqlite3 \\" -ForegroundColor Cyan
    Write-Host "          mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-spdlog" -ForegroundColor Cyan
    Write-Host ""
    if (-not $mingwInPath -and $msys2Root) {
        Write-Host "Also add to PATH:" -ForegroundColor Yellow
        Write-Host "  $(Join-Path $msys2Root 'bin')" -ForegroundColor Cyan
    }
}

Write-Host ""
