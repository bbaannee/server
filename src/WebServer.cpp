#include "WebServer.h"
#include <sys/socket.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <unistd.h>

// --- SocketDeleter ---

void SocketDeleter::operator()(int* fd) const {
    if (fd) {
        Logger::getInstance().log("RAII: Closing socket " + std::to_string(*fd));
        close(*fd);
        delete fd;
    }
}

// --- WebServer ---

WebServer::WebServer(int port) : server(port), port(port), stopServer(false) {}

WebServer::~WebServer() {
    stopServer = true;
    condition.notify_all();
    for (auto& t : workerPool) {
        if (t.joinable()) t.join();
    }
}

void WebServer::start() {
    if (!server.init()) {
        Logger::getInstance().log("Critical: Could not initialize server on port " + std::to_string(port));
        return;
    }

    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 2;

    for (unsigned int i = 0; i < numThreads; ++i) {
        workerPool.emplace_back(&WebServer::workerThread, this);
    }

    Logger::getInstance().log("Server started on port " + std::to_string(port) +
                              " with " + std::to_string(numThreads) + " worker threads");

    while (!stopServer) {
        int clientSocket = server.acceptConnection();
        if (clientSocket != -1) {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push(UniqueSocket(new int(clientSocket)));
            condition.notify_one();
        }
    }
}

void WebServer::workerThread() {
    while (!stopServer) {
        UniqueSocket clientSock;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return !taskQueue.empty() || stopServer; });
            if (stopServer && taskQueue.empty()) return;
            clientSock = std::move(taskQueue.front());
            taskQueue.pop();
        }
        handleClient(std::move(clientSock));
    }
}

// --- Helpers ---

static std::string getContentType(const std::string& path) {
    auto ext = [&](const std::string& e) {
        return path.size() >= e.size() &&
               path.substr(path.size() - e.size()) == e;
    };
    if (ext(".html"))                       return "text/html";
    if (ext(".css"))                        return "text/css";
    if (ext(".js"))                         return "application/javascript";
    if (ext(".png"))                        return "image/png";
    if (ext(".jpg") || ext(".jpeg"))        return "image/jpeg";
    if (ext(".gif"))                        return "image/gif";
    if (ext(".webp"))                       return "image/webp";
    if (ext(".svg"))                        return "image/svg+xml";
    if (ext(".ico"))                        return "image/x-icon";
    if (ext(".txt"))                        return "text/plain";
    if (ext(".json"))                       return "application/json";
    return "application/octet-stream";
}

// Prevent directory traversal attacks (e.g. ../../etc/passwd)
static std::string sanitizePath(const std::string& path) {
    std::string clean;
    clean.reserve(path.size());

    for (size_t i = 0; i < path.size(); ++i) {
        // Skip ".." sequences
        if (i + 1 < path.size() && path[i] == '.' && path[i + 1] == '.') {
            ++i;
            continue;
        }
        // Collapse double slashes
        if (path[i] == '/' && !clean.empty() && clean.back() == '/') continue;
        clean += path[i];
    }
    return clean;
}

void WebServer::handleClient(UniqueSocket clientSockPtr) {
    int clientSocket = *clientSockPtr;

    char buffer[4096] = {};
    if (recv(clientSocket, buffer, sizeof(buffer) - 1, 0) <= 0) return;

    std::string method, path, version;
    std::istringstream ss(buffer);
    ss >> method >> path >> version;

    path = sanitizePath(path);
    if (path == "/") path = "/index.html";

    std::vector<char> content = readFile(path);
    std::string contentType = getContentType(path);
    std::string header;

    if (content.empty()) {
        Logger::getInstance().log("404 " + method + " " + path);
        header = "HTTP/1.1 404 Not Found\r\n"
                 "Content-Length: 0\r\n"
                 "Connection: close\r\n\r\n";
        send(clientSocket, header.c_str(), header.size(), 0);
    } else {
        Logger::getInstance().log("200 " + method + " " + path);
        header = "HTTP/1.1 200 OK\r\n"
                 "Content-Type: " + contentType + "\r\n"
                 "Content-Length: " + std::to_string(content.size()) + "\r\n"
                 "Connection: close\r\n\r\n";
        send(clientSocket, header.c_str(), header.size(), 0);
        send(clientSocket, content.data(), content.size(), 0);  // binary safe
    }
}

std::vector<char> WebServer::readFile(const std::string& fileName) {
    std::ifstream file("www" + fileName, std::ios::binary);
    if (!file.is_open()) return {};
    return std::vector<char>(std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>());
}
