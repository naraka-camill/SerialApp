#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <queue>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
    #define Sleep(x) usleep(x * 1000)
#endif

class TCPClient {
public:
    TCPClient(const std::string& serverIP, int serverPort);
    ~TCPClient();
    
    bool connectToServer();
    void disconnect();
    bool isConnected() const;
    
    
    
    // 处理并发送队列中的消息，需在非阻塞线程中周期性调用
    void processSendQueue();
    
    // 添加消息到发送队列
    void addToSendQueue(const std::string& message);
    
    // 获取从服务器接收到的消息
    void receiveMessages(std::string &msg);
    
private:
    std::string serverIP;
    int serverPort;
    SOCKET clientSocket;
    std::atomic<bool> connected;
    std::atomic<bool> running;
    
    std::string _recvMessage;
    std::mutex recvMutex;
    std::queue<std::string> sendQueue;
    std::mutex queueMutex;
    
    // 接收消息的线程
    std::thread receiveThread;
    
    void _receiveMessages();
    // 发送消息到服务器
    bool sendMessage(const std::string& message);
    
#ifdef _WIN32
    static bool winsockInitialized;
    static void initializeWinsock();
#endif
};

#endif // TCP_CLIENT_H