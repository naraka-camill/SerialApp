# 跨平台TCP客户端使用说明

## 概述
TCPClient 是一个跨平台的TCP客户端实现，支持Windows和Linux系统。它与现有的TCPServer兼容，并具有类似的设计模式。客户端采用异步消息接收机制，支持消息队列管理，确保线程安全的通信。

## 主要特性
- 跨平台支持（Windows/Linux）
- 线程安全的消息发送和接收
- 消息队列机制
- 自动连接管理
- 异步消息接收线程
- 与TCPServer完全兼容

## 主要接口

### 构造函数
```cpp
TCPClient(const std::string& serverIP, int serverPort);
```
- `serverIP`: 服务器IP地址（如 "127.0.0.1"）
- `serverPort`: 服务器端口号（如 8945）

### 连接管理
```cpp
bool connectToServer();     // 连接到服务器
void disconnect();          // 断开连接
bool isConnected() const;   // 检查连接状态
```

### 消息发送
```cpp
bool sendMessage(const std::string& message);           // 直接发送消息（私有方法）
void addToSendQueue(const std::string& message);       // 添加消息到发送队列（推荐）
void processSendQueue();                               // 处理发送队列中的消息
```

### 消息接收
```cpp
void receiveMessages(std::string &msg);                 // 获取从服务器接收到的消息
```

## 使用示例

### 基本用法
```cpp
#include "tcp/tcp_client.h"

int main() {
    // 创建客户端，连接到本地服务器8945端口
    TCPClient client("127.0.0.1", 8945);
    
    // 连接到服务器
    if (!client.connectToServer()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return -1;
    }
    
    // 发送消息（推荐方式：使用队列）
    client.addToSendQueue("Hello from client!");
    client.processSendQueue();
    
    // 接收消息
    std::string received;
    client.receiveMessages(received);
    if (!received.empty()) {
        std::cout << "Received: " << received << std::endl;
    }
    
    // 断开连接
    client.disconnect();
    return 0;
}
```

### 完整的客户端示例
请参见 `main/client_example.cpp` 文件，该示例展示了：
- 连接到服务器
- 发送多条测试消息
- 接收服务器响应
- 优雅断开连接

### 集成测试示例
请参见 `main/integration_test.cpp` 文件，该示例展示了：
- 同时启动服务器和客户端
- 测试双向通信
- 验证消息传输的可靠性

## 编译和运行

### Windows (MinGW/MSYS2)
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake -G "MinGW Makefiles" ..

# 编译客户端示例
cmake --build . --target example1_client

# 编译集成测试
cmake --build . --target integration_test
```

### Linux
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译客户端示例
make example1_client

# 编译集成测试
make integration_test
```

### 运行示例

#### 1. 运行基本客户端示例
1. 首先启动服务器（在一个终端）：
   ```bash
   ./out/example1 8945
   ```

2. 在另一个终端运行客户端：
   ```bash
   ./out/example1_client
   ```

#### 2. 运行集成测试
集成测试会自动启动服务器和客户端：
```bash
./out/integration_test
```

## 实现细节

### 异步消息接收
- 客户端启动时自动创建接收线程
- 接收线程持续监听服务器消息
- 接收到的消息存储在内部缓冲区中
- 通过 `receiveMessages()` 方法获取消息

### 线程安全
- 使用互斥锁保护消息队列
- 使用原子变量管理连接状态
- 所有公共方法都是线程安全的

### 消息处理流程
1. 客户端调用 `addToSendQueue()` 将消息加入发送队列
2. 调用 `processSendQueue()` 处理队列中的消息
3. 消息通过网络发送到服务器
4. 服务器处理并可能回传消息
5. 客户端的接收线程自动接收消息
6. 通过 `receiveMessages()` 获取接收到的消息

## 跨平台兼容性

### Windows 平台
- 使用 Winsock2 API
- 自动初始化 Winsock
- 链接 `ws2_32` 库

### Linux 平台
- 使用 Berkeley Socket API
- 链接 `pthread` 库
- 支持 POSIX 标准

## 错误处理

### 连接错误
- `connectToServer()` 返回 `false` 表示连接失败
- 常见原因：服务器未启动、端口被占用、网络不通

### 消息发送错误
- `processSendQueue()` 内部处理发送错误
- 连接断开时自动停止发送

### 资源清理
- 析构函数自动调用 `disconnect()`
- 确保所有线程正确终止
- 释放网络资源

## 最佳实践

1. **使用消息队列**：推荐使用 `addToSendQueue()` + `processSendQueue()` 而不是直接发送
2. **定期检查消息**：在主循环中定期调用 `receiveMessages()` 检查接收到的消息
3. **优雅断开**：程序结束前调用 `disconnect()` 确保资源正确释放
4. **错误处理**：始终检查 `connectToServer()` 的返回值
5. **线程安全**：可以在多线程环境中安全使用客户端对象

## 注意事项
- 客户端默认端口为 8945（与服务器保持一致）
- 接收消息的线程在连接建立时自动启动
- 消息队列没有大小限制，实际使用中可能需要控制
- Windows 平台下首次使用会自动初始化 Winsock
- 记得在程序结束时调用 `disconnect()` 以正确清理资源