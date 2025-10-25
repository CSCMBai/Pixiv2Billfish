# 快速开始指南

## 1. 安装依赖

### Windows

使用 vcpkg 包管理器（推荐）：

```powershell
# 自动安装脚本已包含在 build.ps1 中
# 直接运行即可
.\build.ps1
```

手动安装：
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl:x64-windows sqlite3:x64-windows nlohmann-json:x64-windows spdlog:x64-windows
```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
    libcurl4-openssl-dev libsqlite3-dev \
    nlohmann-json3-dev libspdlog-dev

# 或使用自动脚本
chmod +x build.sh
./build.sh
```

### macOS

```bash
brew install cmake curl sqlite nlohmann-json spdlog

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## 2. 编译

### Windows

```powershell
# 自动编译脚本
.\build.ps1

# 或手动编译
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

### Linux/macOS

```bash
# 自动编译脚本
chmod +x build.sh
./build.sh

# 或手动编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 3. 配置

编辑 `config.json`:

```json
{
  "db_path": "C:\\path\\to\\your\\billfish.db",  // Windows 路径
  "use_proxies": false,
  "write_tag": true,
  "write_note": true,
  "skip_existing": true,
  "tag_thread_count": 8,
  "note_thread_count": 8
}
```

**重要提示**：
- Windows 路径使用双反斜杠 `\\` 或单正斜杠 `/`
- 数据库路径示例: `C:\\Users\\YourName\\Pictures\\.bf\\billfish.db`
- 处理前务必备份数据库！

## 4. 运行

### Windows
```powershell
cd build\Release
.\Pixiv2Billfish.exe

# 或指定配置文件
.\Pixiv2Billfish.exe ..\..\config.json
```

### Linux/macOS
```bash
cd build
./Pixiv2Billfish

# 或指定配置文件
./Pixiv2Billfish ../config.json
```

## 5. 性能调优

### 线程数设置
```json
"tag_thread_count": 8,    // 建议：CPU核心数 × 1.5
"note_thread_count": 8
```

查看CPU核心数：
- Windows: `系统信息` 或 `echo %NUMBER_OF_PROCESSORS%`
- Linux: `nproc` 或 `lscpu`
- macOS: `sysctl -n hw.ncpu`

### 请求延迟
```json
"request_delay_ms": 100   // 太快可能被限流，建议 100-200ms
```

### 批量大小
如需调整，在 `config.h` 中修改：
```cpp
int batch_size_tag = 20;       // 标签批量大小
int batch_size_tag_join = 50;  // 关联批量大小
int batch_size_note = 10;      // 备注批量大小
```

## 6. 代理设置

如需使用代理：
```json
{
  "use_proxies": true,
  "http_proxy": "http://127.0.0.1:7890",
  "https_proxy": "http://127.0.0.1:7890"
}
```

## 7. 增量处理

处理特定范围的文件：
```json
{
  "start_file_num": 5000,   // 从第 5001 个文件开始
  "end_file_num": 1000      // 处理 1000 个文件
}
```

## 8. 跳过已处理

```json
{
  "skip_existing": true     // 跳过已有标签/备注的文件
}
```

## 9. 日志查看

程序运行时会生成日志文件 `pixiv2billfish.log`，包含详细的处理信息。

## 10. 常见问题

### Q: 编译错误 "找不到 curl.h"
A: 确保已安装 libcurl 开发包，并正确配置 CMake

### Q: 运行时错误 "无法打开数据库"
A: 检查数据库路径是否正确，Windows 路径需要使用 `\\` 或 `/`

### Q: 为什么比 Python 版本快？
A: 参见 `PERFORMANCE.md` 了解详细性能对比

### Q: 内存占用过高？
A: 减小批量大小和线程数

### Q: 请求失败率高？
A: 增大 `request_delay_ms` 和 `retry_count`

### Q: CPU 利用率不高？
A: 增加线程数，但不要超过 CPU 核心数的 2 倍

## 11. 性能基准

参考性能（Intel i7-10700K, 8C16T）：

| 数据量 | 耗时 | 速度 |
|-------|------|-----|
| 500 张 | ~18秒 | ~28张/秒 |
| 2000 张 | ~1.2分钟 | ~27张/秒 |
| 10000 张 | ~6分钟 | ~28张/秒 |

## 12. 安全建议

1. ✅ 处理前备份 Billfish 数据库
2. ✅ 关闭 Billfish 应用
3. ✅ 首次使用时先小范围测试
4. ✅ 注意网络请求频率，避免被限流

## 13. 更多信息

- 性能对比: `PERFORMANCE.md`
- 项目主页: `README.md`
- 问题反馈: GitHub Issues
