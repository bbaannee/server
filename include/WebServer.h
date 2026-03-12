#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "TcpListener.h"
#include "Logger.h"

// RAII wrapper - automatically closes socket when it goes out of scope
struct SocketDeleter {
    void operator()(int* fd) const;
};
using UniqueSocket = std::unique_ptr<int, SocketDeleter>;

class WebServer {
public:
    explicit WebServer(int port);
    ~WebServer();

    void start();

private:
    void workerThread();
    void handleClient(UniqueSocket clientSockPtr);
    std::vector<char> readFile(const std::string& fileName);  // binary safe

    TcpListener server;
    int port;
    bool stopServer;

    // Thread pool
    std::vector<std::thread> workerPool;
    std::queue<UniqueSocket> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
};

#endif
