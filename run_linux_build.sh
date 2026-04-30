#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "脚本所在目录: $SCRIPT_DIR"
cd $SCRIPT_DIR
cd ./build/

# 检查文件是否存在
if [ -f "Makefile" ]; then
    echo "Makefile存在,执行编译..."
    make -j8
else
    echo "Makefile不存在,请先使用QtCreator或者qmake进行配置..."
fi

makeRet=$?

echo "执行结果: ${makeRet}"

if [ ${makeRet} -eq 0 ]; then
    ./Serial
fi