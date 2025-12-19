# Serial 串口通信工具

## 项目概述

这是一个基于 Qt 5.15.2 开发的跨平台串口通信应用程序，提供图形化界面用于串口设备的数据收发。项目使用 C++17 标准，集成了第三方串口通信库，支持 Windows、Linux 和 macOS 平台。

### 主要技术栈

- **框架**: Qt 5.15.2 (Core, GUI, Widgets)
- **语言**: C++17
- **构建系统**: qmake (.pro 文件)
- **编译器**: MinGW 64-bit (Windows)

### 核心功能

1. **串口通信**
   - 自动检测并列出可用串口设备
   - 支持配置波特率、数据位、停止位、校验位
   - 异步读写操作（使用独立线程）
   - 实时数据收发显示

2. **TCP 服务器** (预留功能)
   - 包含 TCP 服务器实现代码
   - 支持多客户端连接
   - 消息广播功能

## 项目架构

```
Serial/
├── main.cpp              # 程序入口
├── app.h/cpp/ui          # 主窗口类（串口通信界面）
├── Serial.pro            # Qt 项目配置文件
├── src/
│   ├── 3rdParty/         # 第三方库
│   │   ├── serial/       # 跨平台串口通信库
│   │   ├── nlohmann/     # JSON 解析库
│   │   └── tinyxml2/     # XML 解析库
│   └── tcp/              # TCP 服务器模块
└── build/                # 构建输出目录
```

### 关键组件

#### App 类 (app.h/cpp)
主应用窗口，负责：
- 串口设备枚举和选择
- 串口参数配置（波特率、数据位等）
- 多线程数据收发
- UI 更新和用户交互

#### 第三方库
- **serial**: 跨平台串口通信库，支持 Windows/Linux/macOS
- **nlohmann/json**: 现代 C++ JSON 库
- **tinyxml2**: 轻量级 XML 解析器
- **TCPServer**: 自实现的 TCP 服务器类

## 构建和运行

### 环境要求

- Qt 5.15.2 或更高版本
- MinGW 64-bit 编译器（Windows）或 GCC/Clang（Linux/macOS）
- qmake 构建工具

### 构建步骤

#### 使用 Qt Creator（推荐）

1. 打开 Qt Creator
2. 选择 "文件" -> "打开文件或项目"
3. 选择 `Serial.pro` 文件
4. 配置构建套件（Kit）
5. 点击左下角的 "构建" 按钮或按 `Ctrl+B`
6. 点击 "运行" 按钮或按 `Ctrl+R` 启动应用

#### 使用命令行

```bash
# 生成 Makefile
qmake Serial.pro

# 编译项目
make  # Linux/macOS
mingw32-make  # Windows (MinGW)

# 运行程序
./Serial  # Linux/macOS
.\Serial.exe  # Windows
```

### 调试

```bash
# Qt Creator 中按 F5 进入调试模式
# 或使用命令行
qmake CONFIG+=debug Serial.pro
make
gdb ./Serial  # Linux/macOS
```

## 开发约定

### 代码风格

- **命名规范**:
  - 类名: 大驼峰 (PascalCase)，如 `App`, `TCPServer`
  - 变量/函数: 小驼峰 (camelCase)，如 `allPorts`, `setEnPortEdit()`
  - 成员变量: 无特殊前缀
  
- **线程安全**:
  - 使用 `std::mutex` 保护共享数据
  - 串口读写操作在独立线程中执行
  - UI 更新通过 Qt 信号槽机制或定时器

- **Qt 特性**:
  - 使用 Qt 信号槽机制处理事件
  - Lambda 表达式用于简单回调
  - 智能指针管理 UI 对象

### 线程模型

项目使用以下线程结构：
1. **主线程**: UI 渲染和事件处理
2. **写线程** (`writeSerial`): 处理串口数据发送
3. **读线程** (`readSerial`): 处理串口数据接收
4. **定时器**: 100ms 周期更新 UI 显示

### 第三方库集成

- 所有第三方库位于 `src/3rdParty/` 目录
- 通过 `INCLUDEPATH` 添加到项目
- 直接包含源码而非动态链接

### 平台兼容性

- Windows: 使用 `setupapi.lib` 进行串口枚举
- Linux/macOS: 使用 POSIX 串口 API
- 条件编译处理平台差异（见 `serial/impl/` 目录）

## Git 仓库

- **远程地址**: https://gitee.com/wuhj2001/qt_serial.git
- **当前分支**: master
- **忽略文件**: build/, .qtcreator/, .vscode/

## 常见任务

### 添加新的串口功能

1. 在 `app.h` 中声明新方法
2. 在 `app.cpp` 中实现逻辑
3. 如需 UI 修改，编辑 `app.ui`（使用 Qt Designer）
4. 注意线程安全，必要时使用互斥锁

### 修改 UI 布局

1. 使用 Qt Creator 打开 `app.ui`
2. 在 Design 模式下拖拽组件
3. 设置信号槽连接
4. 重新编译运行

### 添加新的第三方库

1. 将库文件放入 `src/3rdParty/库名/`
2. 在 `Serial.pro` 中添加头文件和源文件路径
3. 更新 `INCLUDEPATH`
4. 重新运行 qmake

## 注意事项

- 串口操作需要相应的系统权限（Linux 下可能需要 `sudo` 或加入 `dialout` 组）
- Windows 平台需要链接 `setupapi.lib`
- 多线程访问串口对象时注意同步
- UI 更新必须在主线程中进行

## 待办事项

- [ ] 实现 TCP 服务器与串口的数据转发
- [x] 添加数据格式化显示（HEX/ASCII）
- [ ] 实现数据保存到文件功能
- [ ] 添加自动重连机制
- [ ] 完善错误处理和日志记录
