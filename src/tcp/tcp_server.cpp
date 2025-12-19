#include "tcp_server.h"

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cstring>
#endif

#ifdef _WIN32
bool TCPServer::winsockInitialized = false;

void TCPServer::initializeWinsock() {
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

TCPServer::TCPServer(int port) : port(port), serverSocket(INVALID_SOCKET), running(false) {
    std::cout << "Init Tcp Port: " << port << std::endl;
#ifdef _WIN32
    initializeWinsock();
#endif
}

TCPServer::~TCPServer() {
    stop();
}

bool TCPServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 
#ifdef _WIN32
                   (const char*)&opt, 
#else
                   &opt, 
#endif
                   sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Failed to set socket options" << std::endl;
        closesocket(serverSocket);
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(serverSocket);
        return false;
    }
    
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(serverSocket);
        return false;
    }
    
    running = true;
    std::cout << "Server started on port " << port << std::endl;
    
    std::thread acceptThread(&TCPServer::acceptConnections, this);
    acceptThread.detach();
    
    return true;
}

void TCPServer::stop() {
    running = false;
    
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (SOCKET client : clientSockets) {
        closesocket(client);
    }
    clientSockets.clear();
    
#ifdef _WIN32
    if (winsockInitialized) {
        WSACleanup();
        winsockInitialized = false;
    }
#endif
}

void TCPServer::acceptConnections() {
    while (running) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            if (running) {
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }
        
        #ifdef _WIN32
        char* clientIP = inet_ntoa(clientAddr.sin_addr);
#else
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
#endif
        std::cout << "New client connected: " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }
        
        std::thread clientThread(&TCPServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void TCPServer::handleClient(SOCKET clientSocket) {
    char buffer[4096];
    
    while (running) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            break;
        }
        
        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        
        // std::cout << "Received message: " << message << std::endl;
        // broadcastMessage(message); // 接收到消息后回传相同消息
        {
            std::lock_guard<std::mutex> lock(recvMutex);
            recvMessage.append(message);
        }
    }
    
    removeClient(clientSocket);
}

void TCPServer::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    std::vector<SOCKET>::iterator it = clientSockets.begin();
    while (it != clientSockets.end()) {
        SOCKET client = *it;
        int result = send(client, message.c_str(), message.length(), 0);
        
        if (result == SOCKET_ERROR) {
            std::cerr << "Failed to send message to client" << std::endl;
            closesocket(client);
            it = clientSockets.erase(it);
        } else {
            ++it;
        }
    }
}

void TCPServer::removeClient(SOCKET clientSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
    if (it != clientSockets.end()) {
        clientSockets.erase(it);
    }
    
    closesocket(clientSocket);
    std::cout << "Client disconnected" << std::endl;
}



void TCPServer::sndQueueMsg(const std::string &msg)
{
    if (msg.empty()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        sendMsgQueue.push(msg);
    }
}


void TCPServer::receiveMessages(std::string &message)
{
    if (recvMessage.empty()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(recvMutex);
        message.append(recvMessage);
        recvMessage.clear();
    }
}


void TCPServer::processSendMsg()
{
    if (sendMsgQueue.empty()) {
        return;
    }
    
    std::string msg;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        msg = sendMsgQueue.front();
        sendMsgQueue.pop();
    }
    broadcastMessage(msg);
    // std::cout << "Broadcasted: " << msg << std::endl;
}



