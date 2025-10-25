# MinGW 编译说明

## 方式 1: 使用 vcpkg 管理依赖（推荐）

```powershell
.\build-mingw.ps1
```

这个脚本会：
1. 检查 MinGW 是否已安装
2. 自动下载和配置 vcpkg
3. 安装所有依赖库（MinGW 版本）
4. 使用 MinGW 编译项目

## 方式 2: 手动安装依赖

### 安装 MinGW-w64

1. 下载 MinGW-w64: https://www.mingw-w64.org/downloads/
2. 推荐使用 MSYS2:
   ```powershell
   # 下载并安装 MSYS2
   # https://www.msys2.org/
   
   # 在 MSYS2 终端中安装工具链和依赖
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-cmake
   pacman -S mingw-w64-x86_64-curl
   pacman -S mingw-w64-x86_64-sqlite3
   pacman -S mingw-w64-x86_64-nlohmann-json
   pacman -S mingw-w64-x86_64-spdlog
   ```

3. 将 MinGW bin 目录添加到 PATH:
   - MSYS2: `C:\msys64\mingw64\bin`
   - 或其他 MinGW 安装路径

### 编译

```powershell
.\build-mingw-quick.ps1
```

## 方式 3: 完全手动编译

```powershell
# 创建构建目录
mkdir build-mingw
cd build-mingw

# 配置
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 编译
mingw32-make -j%NUMBER_OF_PROCESSORS%

# 可执行文件
.\Pixiv2Billfish.exe
```

## 检查 MinGW 安装

```powershell
# 检查 g++
g++ --version

# 检查 make
mingw32-make --version

# 检查 cmake
cmake --version
```

## MSVC vs MinGW

| 特性 | MSVC | MinGW |
|------|------|-------|
| 编译器 | Microsoft Visual C++ | GCC |
| 开源 | ❌ | ✅ |
| 跨平台兼容性 | Windows Only | 更好 |
| 调试工具 | Visual Studio | GDB |
| 性能 | 略好 | 相近 |
| C++标准支持 | 完整 | 完整 |

## 常见问题

### Q: 找不到 g++
A: 确保 MinGW bin 目录在 PATH 中
```powershell
$env:PATH += ";C:\msys64\mingw64\bin"
```

### Q: 找不到 mingw32-make
A: 如果使用 MSYS2，命令可能是 `make` 而不是 `mingw32-make`

### Q: 缺少依赖库
A: 使用 vcpkg 脚本或在 MSYS2 中用 pacman 安装

### Q: 编译错误 undefined reference
A: 确保所有依赖库都已正确安装并在 CMake 中找到

## 性能对比

MinGW 编译的程序性能与 MSVC 相近，在某些情况下甚至更好：
- 启动时间: 相近
- 运行速度: 相近
- 内存占用: 略低
- 可执行文件大小: 通常更小

## 推荐配置

对于 Pixiv2Billfish 项目，推荐使用 **MSYS2 + MinGW-w64**：
1. 依赖管理方便（pacman）
2. 工具链完整
3. 更新及时
4. 社区支持好
