
#!/bin/bash
#获取当前脚本所在目录作为项目根目录

PROJECT_ROOT=$(cd `dirname $0` && pwd)
BUILD_DIR="$PROJECT_ROOT/build/Debug"

echo "=== 清理旧构建缓存 ==="
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR" && echo "已删除旧构建目录"
fi

echo -e "\n=== 生成新构建系统 ==="
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/gcc-arm-none-eabi.cmake" \
      -S "$PROJECT_ROOT" \
      -B "$BUILD_DIR" \
      -G Ninja

echo -e "\n=== 开始编译 ==="
cmake --build "$BUILD_DIR"