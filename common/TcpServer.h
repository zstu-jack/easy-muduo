#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "TcpConnection.h"
#include "Define.h"

#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Socket;
class EventLoop;

class TcpServer{
public:
    TcpServer(EventLoop* loop, int listen_port);
    ~TcpServer();
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

public:
    void start(); // add to event loop.

public:
    void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb){ messageCallback_ = cb; }
    void setPkgDecodeCallback(const PkgDecodeCallback& cb){ pkgDecodeCallback_ = cb; }

private:
    void newConnection();
    void removeConnection(const TcpConnection* conn);
    void impossibleEvent();

private:
    EventLoop* loop_;
    int listen_port_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    PkgDecodeCallback pkgDecodeCallback_;
    std::map<int , TcpConnection*> connectionMap_;   // fd->
    Socket* socket;
};

#endif