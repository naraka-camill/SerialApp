#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>
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
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

class TCPServer {
public:
    TCPServer(int port);
    ~TCPServer();
    
    bool start();
    void stop();
    
    // 处理并广播队列中的消息，需在非阻塞线程中周期性调用
    void processSendMsg();
    // 发送消息到队列，由非阻塞线程"processSendMsg()"处理并广播
    void sndQueueMsg(const std::string &msg);
    // 接收所有客户端发送的消息,将消息追加到参数message中
    void receiveMessages(std::string &message);
    
private:

    int port;
    SOCKET serverSocket;
    std::vector<SOCKET> clientSockets;
    std::mutex clientsMutex;
    std::atomic<bool> running;

    std::string recvMessage;
    std::mutex recvMutex;
    std::queue<std::string> sendMsgQueue;
    std::mutex queueMutex;
    
    void acceptConnections();
    void handleClient(SOCKET clientSocket);
    void removeClient(SOCKET clientSocket);
    void broadcastMessage(const std::string &message);
    
#ifdef _WIN32
    static bool winsockInitialized;
    static void initializeWinsock();
#endif
};

#endif // TCP_SERVER_H