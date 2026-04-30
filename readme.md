# Serial 串口调试助手

![Version](https://img.shields.io/badge/version-v1.3.1-blue.svg)
[![License](https://img.shields.io/badge/license-AGPL%20v3-blue.svg)](LICENSE.txt)
[![Qt Version](https://img.shields.io/badge/Qt-5.15.2-41CD52.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)]()
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

一个基于 Qt 5.15.2 的跨平台串口通信调试工具，提供图形化界面用于串口设备的数据收发与调试。

## ✨ 特性

- 🔌 **串口动态扫描** — 周期性检测串口插拔，实时刷新设备列表
- 🔁 **自动重连** — 串口意外断开时自动尝试恢复连接
- ⚙️ **完整参数配置** — 波特率、数据位、停止位、校验位
- ⌨️ **回车直接发送** — 输入完数据按 Enter 即可发送
- 🛡️ **参数校验** — 打开串口前检查配置有效性，防止崩溃
- 🔄 **多线程通信** — 独立读写线程，界面不卡顿
- 📊 **ASCII / HEX 双格式** — 实时收发，格式切换
- 💾 **配置持久化** — 上次使用的串口参数自动保存
- 🌐 **TCP 扩展模块** — 预留 TCP 客户端 / 服务端
- 🎨 **跨平台** — Windows (MinGW) + Linux (GCC)

## 🏗️ 项目结构

```
Serial/
├── main.cpp                   # 入口
├── app.h / app.cpp / app.ui   # 主窗口
├── Serial.pro                 # qmake 项目文件
├── run_win_build.sh           # Windows 构建脚本
├── run_linux_build.sh         # Linux 构建脚本
├── setup.iss                  # Inno Setup 打包 (Windows)
├── windows_icon.rc            # 应用图标
├── debian-package/            # deb 打包 (Linux)
│   └── DEBIAN/
│       ├── control
│       ├── postinst
│       └── prerm
├── src/
│   ├── 3rdParty/
│   │   ├── serial/            # 跨平台串口库
│   │   ├── nlohmann/          # JSON 解析
│   │   └── tinyxml2/          # XML 解析
│   └── tcp/                   # TCP 模块
│       ├── tcp_client.cpp/h
│       └── tcp_server.cpp/h
├── qrc/icon/                  # 图标资源
└── build/                     # 构建输出
```

## 快速使用

下载最新版本的安装包：
https://github.com/naraka-camill/SerialApp/releases


## 🚀 构建

### 环境

| 平台 | 编译器 | Qt |
|------|--------|----|
| Windows | MinGW 64-bit | 5.15.2 |
| Linux | GCC 7.0+ | 5.15.2 |

### 方式一：Qt Creator

1. 打开 `Serial.pro` → 配置 Kit → 构建 (Ctrl+B) → 运行 (Ctrl+R)

### 方式二：命令行

```bash
qmake Serial.pro
# Windows
mingw32-make -j
# Linux
make -j8
```

### 方式三：构建脚本

```bash
# Windows (Git Bash)
./run_win_build.sh

# Linux
./run_linux_build.sh    # 编译成功自动运行
```

### 打包

```bash
# Windows — Inno Setup 编译 setup.iss 生成安装包
# Linux — dpkg-deb --build debian-package
```

## 💻 使用说明

### 串口操作

1. 程序自动扫描可用串口，设备插拔实时刷新
2. 选择串口号、配置波特率/数据位/停止位/校验位
3. 点击连接，状态栏显示连接成功/失败
4. 输入数据，点击发送或直接按 Enter 发送
5. ASCII / HEX 即时切换

### 自动重连

连接意外断开时，程序会持续尝试重连，无需手动干预。

### 配置持久化

关闭程序时自动保存当前串口参数到 `.cfg` 文件，下次启动自动恢复。

## 🧵 线程模型

| 线程 | 职责 | 周期 |
|------|------|------|
| 主线程 | UI 渲染、用户交互 | — |
| 写线程 `writeSerial` | 串口数据发送 | 20ms |
| 读线程 `readSerial` | 串口数据接收 | 20ms |
| UI 更新定时器 | 刷新数据显示 | 20ms |
| 串口扫描器 | 检测端口变化 | 1s |

互斥锁 `serMutex` / `recMutex` / `sendMutex` 保护共享资源，防止竞态。

## 🔍 故障排除

**串口打不开**
- 检查是否被其他程序占用
- 确认驱动已安装
- Linux 用户需加入 `dialout` 组

**编译失败**
- 确认 Qt / MinGW 环境变量正确
- 先运行 qmake 生成 Makefile

## 📜 更新日志

### v1.3.1
- 🆕 串口动态扫描 — 周期性检测串口插拔并刷新列表
- 🔁 串口自动重连 — 断开后自动尝试恢复连接
- ⚡ 通用构建脚本 — Windows / Linux 统一脚本
- ⚡ 多线程编译 — 脚本使用 `-j` 并行编译加速

### v1.3.0
- 🚀 版本号升版至 v1.3.0
- 🛡️ Win 端口号默认不可编辑，避免无效输入
- ⌨️ 新增回车直接发送数据
- 🔒 修复关闭串口时的竞态条件崩溃（加入互斥锁保护）
- 📝 优化调试日志
- ⚡ 降低轮询频率，减少锁竞争，UI 更新更流畅
- ✅ 打开串口时检查参数设置，防止崩溃

### v1.2.0
- 🔧 调整 `.pro` 文件自动生成，不再纳入版本管理
- ⚡ 多线程编译支持
- 📦 新增 Inno Setup 安装包打包 (Windows)
- 🐧 新增 deb 包打包脚本 (Linux)
- 🐧 Linux 环境适配（权限、依赖）
- 💻 多平台编译脚本（`run_win_build.sh` / `run_linux_build.sh`）

### v1.1.0
- 💾 基础配置持久化 — 启动自动读取，关闭自动保存 `.cfg`
- 🛠️ 修复串口配置问题，优化代码结构
- 💡 增加 ToolTip 提示
- 🔢 HEX 输入格式化与合法性检查
- 📶 串口开关状态文本显示
- ⏱️ 提高收发刷新率，通信更灵敏
- 🕒 数据接收增加时间戳显示
- 🏷️ 窗口标题显示版本号
- 🎨 应用图标与署名

### v1.0.0
- 🎉 初始版本
- 串口枚举、参数配置、连接/断开
- ASCII / HEX 双格式收发
- 多线程读写，界面不阻塞
- 3rdParty 集成：serial、nlohmann/json、tinyxml2
- TCP 客户端/服务端模块定义
- README 与开源协议

---

[GNU AFFERO GENERAL PUBLIC LICENSE Version 3](LICENSE.txt)
