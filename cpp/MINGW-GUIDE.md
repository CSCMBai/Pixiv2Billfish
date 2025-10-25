# MinGW 编译完整指南

## 推荐方式：使用 MSYS2

### 1. 安装 MSYS2

下载并安装：https://www.msys2.org/

### 2. 安装依赖包

打开 **MSYS2 MinGW 64-bit** 终端，运行：

```bash
# 更新包数据库
pacman -Syu

# 安装编译工具
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make

# 安装依赖库
pacman -S mingw-w64-x86_64-curl
pacman -S mingw-w64-x86_64-sqlite3
pacman -S mingw-w64-x86_64-nlohmann-json
pacman -S mingw-w64-x86_64-spdlog
```

或者一行安装全部：
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make \
          mingw-w64-x86_64-curl mingw-w64-x86_64-sqlite3 \
          mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-spdlog
```

### 3. 添加到 PATH

将 MSYS2 MinGW64 bin 目录添加到系统 PATH：
```
C:\msys64\mingw64\bin
```

### 4. 编译项目

#### 方式 A：在 PowerShell 中
```powershell
.\build-mingw-simple.ps1
```

#### 方式 B：在 CMD 中
```cmd
build-mingw.bat
```

#### 方式 C：在 MSYS2 终端中
```bash
bash build-mingw-msys2.sh
```

#### 方式 D：手动编译
```bash
mkdir build-mingw
cd build-mingw
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 验证安装

在 PowerShell 或 CMD 中运行：

```cmd
g++ --version
cmake --version
mingw32-make --version
```

应该看到版本信息输出。

## 编译选项说明

### 基本编译
```bash
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
make
```

### 并行编译（更快）
```bash
make -j8  # 8 线程并行编译
```

### Debug 版本
```bash
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
make
```

### 详细输出
```bash
make VERBOSE=1
```

## 常见问题

### Q: 找不到 g++
**A:** 确保 MSYS2 的 mingw64/bin 在 PATH 中
```powershell
$env:PATH += ";C:\msys64\mingw64\bin"
```

### Q: cmake 找不到库
**A:** 确保使用 MSYS2 安装的库，不要混用其他来源的库

### Q: 编译错误：undefined reference
**A:** 检查是否所有依赖都已安装：
```bash
pacman -Q | grep mingw-w64-x86_64-curl
pacman -Q | grep mingw-w64-x86_64-sqlite3
pacman -Q | grep mingw-w64-x86_64-nlohmann-json
pacman -Q | grep mingw-w64-x86_64-spdlog
```

### Q: mingw32-make vs make
**A:** 在 MSYS2 中使用 `make`，在 Windows CMD/PowerShell 中使用 `mingw32-make`

### Q: 需要哪个版本的 MinGW？
**A:** 推荐 MinGW-w64 (64位)，不是老的 MinGW (32位)

## 性能对比

使用 MinGW 编译的程序与 MSVC 编译的性能相近：

| 指标 | MSVC | MinGW |
|------|------|-------|
| 编译速度 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 运行速度 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 可执行文件大小 | 较大 | 较小 |
| 跨平台 | ❌ | ✅ |
| 开源 | ❌ | ✅ |

## 提示

1. **首选 MSYS2**：最简单、最可靠的方式
2. **不要混用工具链**：统一使用 MSYS2 的包
3. **PATH 顺序**：MSYS2 路径应在系统 PATH 前面
4. **并行编译**：使用 `-j` 参数加速编译

## 下一步

编译完成后，可执行文件在：
```
build-mingw\Pixiv2Billfish.exe
```

配置和运行方法见主 README.md
