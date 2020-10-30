#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "Define.h"
class EventLoop;

class TcpClient{
public:
    TcpClient(EventLoop *loop, const char* ip, uint16_t port);
    ~TcpClient();
    void connect();
    void disconnect();

    TcpConnection* connection() const {
        return connection_;
    }
    EventLoop *getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setPkgDecodeCallback(const PkgDecodeCallback& cb){ pkgDecodeCallback_ = cb; }
    // void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnection* conn);
    void impossibleEvent(int sockfd);
    void connecting(int sockfd);
    void retry(int sockfd);

    EventLoop *loop_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    PkgDecodeCallback pkgDecodeCallback_;
    // WriteCompleteCallback writeCompleteCallback_;
    bool retry_;
    TcpConnection* connection_ = nullptr;

    std::string peer_ip_;
    uint16_t peer_port_;
};

#endif