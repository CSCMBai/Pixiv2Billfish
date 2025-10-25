#!/bin/bash

# Linux 编译脚本

echo "安装依赖..."

# 检测系统
if [ -f /etc/debian_version ]; then
    # Debian/Ubuntu
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libcurl4-openssl-dev \
        libsqlite3-dev nlohmann-json3-dev libspdlog-dev git
elif [ -f /etc/redhat-release ]; then
    # RedHat/CentOS/Fedora
    sudo yum install -y gcc-c++ cmake libcurl-devel sqlite-devel \
        json-devel spdlog-devel git
elif [ -f /etc/arch-release ]; then
    # Arch Linux
    sudo pacman -S --noconfirm base-devel cmake curl sqlite \
        nlohmann-json spdlog git
else
    echo "不支持的系统，请手动安装依赖"
    exit 1
fi

# 创建构建目录
rm -rf build
mkdir build
cd build

# 配置 CMake
echo "配置 CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
echo "编译中..."
make -j$(nproc)

echo "编译完成！"
echo "可执行文件位于: build/Pixiv2Billfish"

cd ..
