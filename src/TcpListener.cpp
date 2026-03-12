#include "TcpListener.h"
#include <unistd.h>
#include <cstring>

TcpListener::TcpListener(int port) : m_port(port), m_serverSocket(-1) {}

TcpListener::~TcpListener() {
    if (m_serverSocket != -1) {
        close(m_serverSocket);
    }
}

bool TcpListener::init() {
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // IPv4, TCP
    if (m_serverSocket == -1) return false;

    // Allow port reuse immediately after server restart
    int opt = 1;
    setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_serverSocket, (sockaddr*)&hint, sizeof(hint)) == -1) return false;
    if (listen(m_serverSocket, SOMAXCONN) == -1) return false;

    return true;
}

int TcpListener::acceptConnection() {
    sockaddr_in client{};
    socklen_t clientSize = sizeof(client);
    return accept(m_serverSocket, (sockaddr*)&client, &clientSize);
}
