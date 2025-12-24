#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "脚本所在目录: $SCRIPT_DIR"
cd $SCRIPT_DIR
cd ./build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/

# 检查文件是否存在
if [ -f "Makefile" ]; then
    echo "Makefile存在，执行编译..."
    mingw32-make.exe
else
    echo "Makefile不存在，请先使用QtCreator或者qmake进行配置..."
fi
