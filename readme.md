# Serial 串口调试助手

[![Version](https://img.shields.io/badge/version-v1.0.0-blue.svg)](https://gitee.com/wuhj2001/qt_serial)
[![License](https://img.shields.io/badge/license-AGPL%20v3-blue.svg)](LICENSE.txt)
[![Qt Version](https://img.shields.io/badge/Qt-5.15.2-41CD52.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

一个基于 Qt 5.15.2 开发的跨平台串口通信调试工具，提供图形化界面用于串口设备的数据收发和调试。

## 📋 项目概述

本项目是一个功能完善的串口调试助手，支持多平台运行，具有直观的用户界面和强大的串口通信功能。采用 C++17 标准开发，集成了成熟的第三方串口通信库，确保稳定可靠的通信性能。

### ✨ 主要特性

- 🔌 **智能串口管理** - 自动检测系统可用串口设备
- ⚙️ **完整参数配置** - 支持波特率、数据位、停止位、校验位配置
- 🔄 **多线程通信** - 读写操作在独立线程中执行，确保界面流畅
- 📊 **实时数据显示** - 支持ASCII和HEX格式数据收发
- 🌐 **跨平台支持** - 兼容 Windows、Linux 和 macOS 系统
- 🎨 **现代化界面** - 基于 Qt Widgets 的直观用户界面

## 🛠️ 技术架构

### 核心技术栈

- **开发框架**: Qt 5.15.2 (Core, GUI, Widgets)
- **编程语言**: C++17
- **构建系统**: qmake (.pro 配置文件)
- **编译器**: MinGW 64-bit (Windows) / GCC (Linux/macOS)

### 项目结构

```
Serial/
├── main.cpp                    # 程序入口点
├── app.h/cpp/ui               # 主窗口类实现
├── Serial.pro                 # Qt 项目配置文件
├── build.sh                   # Windows 构建脚本
├── windows_icon.rc            # Windows 应用图标配置
├── src/
│   ├── 3rdParty/              # 第三方库集成
│   │   ├── serial/            # 跨平台串口通信库
│   │   ├── nlohmann/          # JSON 解析库
│   │   └── tinyxml2/          # XML 解析库
│   └── tcp/                   # TCP 通信模块（预留）
├── qrc/icon/                  # 应用图标资源
└── build/                     # 构建输出目录
```

### 核心组件

#### App 主窗口类 (`app.h/cpp`)

负责应用程序的主要功能实现：
- **串口设备管理**: 自动枚举和检测可用串口
- **参数配置界面**: 波特率、数据位、停止位、校验位设置
- **多线程通信**: 独立的读写线程处理数据收发
- **UI 更新机制**: 定时器驱动的界面刷新
- **数据格式转换**: ASCII与HEX格式互转

#### 第三方库集成

- **serial**: 成熟的跨平台串口通信库
  - 支持Windows (setupapi)、Linux/macOS (POSIX)
  - 提供统一的串口操作接口
- **nlohmann/json**: 现代C++ JSON处理库
- **tinyxml2**: 轻量级XML解析器

## 🚀 快速开始

### 环境要求

- **Qt Framework**: 5.15.2 或更高版本
- **编译器**: 
  - Windows: MinGW 64-bit
  - Linux: GCC 7.0+
  - macOS: Clang 9.0+
- **构建工具**: qmake

### 安装与构建

#### 方法一：使用 Qt Creator（推荐）

1. 打开 Qt Creator IDE
2. 选择 `文件` → `打开文件或项目`
3. 导航并选择 `Serial.pro` 文件
4. 配置合适的构建套件（Kit）
5. 点击左下角 `构建` 按钮 (Ctrl+B)
6. 点击 `运行` 按钮启动应用 (Ctrl+R)

#### 方法二：命令行构建

**Windows 环境:**
```bash
# 生成 Makefile
qmake Serial.pro

# 编译项目
mingw32-make

# 运行程序
.\build\Desktop_Qt_5_15_2_MinGW_64_bit-Debug\Serial.exe
```

**Linux/macOS 环境:**
```bash
# 生成 Makefile
qmake Serial.pro

# 编译项目
make

# 运行程序
./Serial
```

#### 方法三：使用构建脚本（Windows）

```bash
# 在 Git Bash 中执行
./build.sh
```

> **注意**: 首次构建需要使用 Qt Creator 或 qmake 生成 Makefile 文件

## 💻 功能说明

### 串口连接管理

1. **设备检测**: 应用启动时自动扫描系统可用串口
2. **参数配置**: 
   - 波特率: 9600, 19200, 38400, 57600, 115200 等
   - 数据位: 5, 6, 7, 8
   - 停止位: 1, 1.5, 2
   - 校验位: 无, 奇校验, 偶校验
3. **连接控制**: 点击按钮连接/断开串口设备

### 数据通信

- **发送数据**: 支持ASCII和HEX格式数据发送
- **接收数据**: 实时显示接收到的串口数据
- **清空显示**: 一键清空发送/接收区域
- **多线程处理**: 读写操作在独立线程中执行，不阻塞UI

### 界面布局

应用界面分为以下几个主要区域：
- **连接配置区**: 串口选择和参数设置
- **发送数据区**: 数据输入和发送控制
- **接收数据区**: 实时显示接收数据
- **状态信息区**: 连接状态和统计信息

## 🔧 开发指南

### 代码规范

#### 命名约定
- **类名**: PascalCase (大驼峰) - `App`, `SerialPort`
- **函数/变量**: camelCase (小驼峰) - `setEnPortEdit()`, `receiveMsg`
- **成员变量**: 无特殊前缀，使用有意义的名称

#### 线程安全
- 使用 `std::mutex` 保护共享数据访问
- 串口读写操作在独立线程中执行
- UI更新通过Qt信号槽机制或定时器实现

#### Qt最佳实践
- 优先使用Qt信号槽机制处理事件
- Lambda表达式用于简单的回调函数
- 智能指针管理Qt对象生命周期

### 线程模型

应用程序采用多线程架构：

1. **主线程**: UI渲染和用户交互处理
2. **写线程** (`writeSerial`): 处理串口数据发送
3. **读线程** (`readSerial`): 处理串口数据接收  
4. **UI更新定时器**: 100ms周期更新界面显示

### 扩展开发

#### 添加新功能

1. 在 `app.h` 中声明新的成员函数
2. 在 `app.cpp` 中实现具体逻辑
3. 如需UI修改，使用Qt Designer编辑 `app.ui`
4. 注意线程安全，必要时使用互斥锁保护共享资源

#### 集成新的第三方库

1. 将库文件放置到 `src/3rdParty/库名/` 目录
2. 在 `Serial.pro` 中添加相应的源文件和头文件路径
3. 更新 `INCLUDEPATH` 配置
4. 重新运行qmake生成Makefile

## 🔍 故障排除

### 常见问题

**串口连接失败**
- 检查串口设备是否被其他程序占用
- 确认串口驱动程序已正确安装
- 验证串口参数配置是否正确

**编译错误**
- 确保Qt开发环境配置正确
- 检查MinGW编译器是否在PATH环境变量中
- 验证所有依赖库文件是否存在

**权限问题**
- Linux系统可能需要将用户添加到 `dialout` 组
- 或使用 `sudo` 权限运行程序
- Windows系统确保用户有足够权限访问串口设备

### 调试模式

```bash
# 启用调试信息编译
qmake CONFIG+=debug Serial.pro
make

# 使用调试器启动
gdb ./Serial  # Linux/macOS
```


## 🌐 Git 仓库

- **远程地址**: https://gitee.com/wuhj2001/qt_serial.git
- **默认分支**: master

## 📋 开发计划

### 已完成功能
- [x] 基础串口通信功能
- [x] 多线程数据收发
- [x] ASCII/HEX数据格式支持
- [x] 跨平台兼容性

### 待实现功能
- [ ] 数据保存到文件功能
- [ ] 自动重连机制
- [ ] 更完善的错误处理和日志记录
- [ ] TCP服务器与串口数据转发

## 📄 LICENSE

[GNU AFFERO GENERAL PUBLIC LICENSE Version 3](LICENSE.txt)
