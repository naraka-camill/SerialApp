#include "tcp_client.h"

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <cstring>
#endif

#ifdef _WIN32
bool TCPClient::winsockInitialized = false;

void TCPClient::initializeWinsock() {
    if (!winsockInitialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            exit(1);
        }
        winsockInitialized = true;
    }
}
#endif

TCPClient::TCPClient(const std::string& serverIP, int serverPort) 
    : serverIP(serverIP), serverPort(serverPort), clientSocket(INVALID_SOCKET), 
      connected(false), running(false) {
    std::cout << "Init TCP Client - Server: " << serverIP << ", Port: " << serverPort << std::endl;
#ifdef _WIN32
    initializeWinsock();
#endif
}

TCPClient::~TCPClient() {
    disconnect();
}

bool TCPClient::connectToServer() {
    if (connected) {
        std::cout << "Already connected to server" << std::endl;
        return true;
    }
    
    // 创建套接字
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    // 将IP地址转换为网络字节序
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid server IP address: " << serverIP << std::endl;
        
        // 如果IP地址无效，尝试通过主机名解析
        struct hostent* host = gethostbyname(serverIP.c_str());
        if (host != nullptr) {
            memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);
        } else {
            std::cerr << "Failed to resolve server hostname: " << serverIP << std::endl;
            closesocket(clientSocket);
            return false;
        }
    }
    
    // 连接到服务器
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server: " << serverIP << ":" << serverPort << std::endl;
        closesocket(clientSocket);
        return false;
    }
    
    connected = true;
    running = true;
    std::cout << "Connected to server: " << serverIP << ":" << serverPort << std::endl;
    
    // 启动接收消息的线程
    receiveThread = std::thread(&TCPClient::_receiveMessages, this);
    
    return true;
}

void TCPClient::disconnect() {
    if (!connected) {
        return;
    }
    
    running = false;
    
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    
    // 等待接收线程结束
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    
    connected = false;
    std::cout << "Disconnected from server" << std::endl;
}

bool TCPClient::isConnected() const {
    return connected;
}

bool TCPClient::sendMessage(const std::string& message) {
    if (!connected || clientSocket == INVALID_SOCKET) {
        std::cerr << "Not connected to server" << std::endl;
        return false;
    }
    
    int result = send(clientSocket, message.c_str(), message.length(), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Failed to send message to server" << std::endl;
        connected = false;
        return false;
    }
    
    return true;
}

void TCPClient::addToSendQueue(const std::string& message) {
    if (message.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(queueMutex);
    sendQueue.push(message);
}

void TCPClient::processSendQueue() {
    if (sendQueue.empty() || !connected) {
        return;
    }
    
    std::string message;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!sendQueue.empty()) {
            message = sendQueue.front();
            sendQueue.pop();
        }
    }
    
    if (!message.empty()) {
        sendMessage(message);
    }
}

void TCPClient::_receiveMessages() {
    char buffer[4096];
    
    while (running && connected) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            if (running) {
                std::cerr << "Connection to server lost" << std::endl;
            }
            connected = false;
            break;
        }
        
        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        
        // 将接收到的消息添加到接收缓冲区
        {
            std::lock_guard<std::mutex> lock(recvMutex);
            _recvMessage.append(message);
        }
        
        std::cout << "Received from server: " << message << std::endl;
    }
}

void TCPClient::receiveMessages(std::string &msg) {
    if (_recvMessage.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(recvMutex);
    msg = _recvMessage;
    _recvMessage.clear();
}