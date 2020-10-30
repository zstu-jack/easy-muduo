#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <string>

#include "EventLoop.h"
#include "Socket.h"

const int socket_buff_size = 10240;

class TcpConnection{
public:
    TcpConnection(EventLoop* loop, int sockfd, struct sockaddr_in& peerAddr);
    ~TcpConnection();
    const struct sockaddr_in& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    void send(const void* message, int len);
    void send(const std::string& message);
    void forceClose();
    void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb){ messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb){ closeCallback_ = cb; }
    void setPkgDecodeCallback(const PkgDecodeCallback & cb){  pkgDecodeCallback_ = cb; }
    // void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = cb; }

    void handleRead();
    void handleClose();
    void handleWrite();
    void handleError();


public:
    int get_fd() const;
private:
    // common.
    EventLoop* loop_;
    Socket* socket_;
    enum StateE { kDisconnected, kConnected, kConnecting };
    StateE state_;
    struct sockaddr_in peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    // WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    PkgDecodeCallback pkgDecodeCallback_;

    char read_buf[socket_buff_size];
    int read_buf_index = 0;
    // for server side.


    // for client side.
};


#endif