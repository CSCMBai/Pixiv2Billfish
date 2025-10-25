# Pixiv2Billfish C++ 高性能版本

这是 Pixiv2Billfish 的 C++ 重写版本，相比 Python 版本具有以下优势：

## 性能优化

1. **多线程并发**: 使用现代 C++ 线程池实现真正的并行处理
2. **内存管理**: 零拷贝设计，减少内存分配和释放开销
3. **数据库优化**: 
   - 批量写入事务，减少 I/O 操作
   - 连接池复用，避免频繁创建连接
   - 内存缓存，减少重复查询
4. **网络优化**: 
   - 异步 HTTP 请求
   - 连接复用
   - 智能重试机制
5. **编译优化**: 使用 -O3 优化和 CPU 指令集优化

## 性能提升

预计相比 Python 版本性能提升：
- **速度**: 3-5倍
- **内存占用**: 减少 50-70%
- **CPU 利用率**: 更高效的多核利用

## 依赖库

- **CURL**: HTTP 客户端
- **SQLite3**: 数据库
- **nlohmann/json**: JSON 解析
- **spdlog**: 日志库

## 编译

### Windows (MSVC)

```powershell
# 安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 安装依赖
.\vcpkg install curl sqlite3 nlohmann-json spdlog

# 编译
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

### Linux (GCC/Clang)

```bash
# 安装依赖 (Ubuntu/Debian)
sudo apt-get install libcurl4-openssl-dev libsqlite3-dev nlohmann-json3-dev libspdlog-dev

# 编译
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 使用

1. 编辑 `config.json` 配置文件
2. 运行程序：

```bash
# Windows
.\Pixiv2Billfish.exe

# Linux
./Pixiv2Billfish
```

## 配置说明

```json
{
  "db_path": "billfish.db",           // 数据库路径
  "use_proxies": false,                // 是否使用代理
  "http_proxy": "http://127.0.0.1:7890",
  "https_proxy": "http://127.0.0.1:7890",
  "write_tag": true,                   // 写入标签
  "write_note": true,                  // 写入备注
  "skip_existing": true,               // 跳过已有数据
  "start_file_num": 0,                 // 起始文件序号
  "end_file_num": 0,                   // 结束文件序号（0=全部）
  "tag_thread_count": 8,               // 标签处理线程数
  "note_thread_count": 8,              // 备注处理线程数
  "request_timeout": 5,                // 请求超时（秒）
  "retry_count": 5,                    // 重试次数
  "request_delay_ms": 100              // 请求间隔（毫秒）
}
```

## 性能调优建议

1. **线程数**: 根据 CPU 核心数调整，建议设置为核心数的 1-2 倍
2. **批量大小**: 内存充足时可增大批量写入数量
3. **请求延迟**: 避免过于频繁的请求被 Pixiv 限流
4. **数据库**: 处理前备份，处理时关闭 Billfish 应用

## 技术特性

- **C++17 标准**: 现代 C++ 特性
- **RAII 资源管理**: 自动内存管理
- **异常安全**: 完善的异常处理
- **线程安全**: 无锁数据结构和互斥锁保护
- **智能指针**: 避免内存泄漏

## 许可证

与原 Python 版本相同
