#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <sys/socket.h>
#include <netinet/in.h>

class TcpListener {
public:
    explicit TcpListener(int port);
    ~TcpListener();

    // Non-copyable
    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    bool init();
    int acceptConnection();

private:
    int m_port;
    int m_serverSocket;
};

#endif
