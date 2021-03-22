#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "Define.h"
class EventLoop;

class TcpClient{
public:
    TcpClient(EventLoop *loop, const char* ip, uint16_t port);
    ~TcpClient();
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    void connect();
    void disconnect();
    bool connected();

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

public:


private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnection* conn);
    void impossibleReadEvent(int sockfd);
    void impossibleErrorEvent(int sockfd);
    void connecting(int sockfd);
    void retry(int sockfd);

    EventLoop *loop_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    PkgDecodeCallback pkgDecodeCallback_;
    // WriteCompleteCallback writeCompleteCallback_;

    TcpConnection* connection_ = nullptr;

    // Connector.
    bool retry_;
    int sockfd;
    int connect_;   // = 1 for user wanna connect, 0 for user disconnect.
    StateE state_;

    std::string peer_ip_;
    uint16_t peer_port_;
};

#endif