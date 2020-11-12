#include "TcpClient.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include <cstdio>
#include <cstring>

TcpClient::TcpClient(EventLoop* loop, const char* ip, uint16_t port)
        : loop_(loop),
          retry_(true){
    peer_ip_ = std::string(ip);
    peer_port_ = port;
    connection_ = nullptr;
}

TcpClient::~TcpClient()
{
    if (connection_)
    {
       removeConnection(connection_);
    }
}

void TcpClient::connect()
{
    if (connection_)
    {
        return ;
    }


    // client fd.
    int sockfd = createNonBlockSocket();
    // peer addr.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_aton(peer_ip_.c_str(), &addr.sin_addr);
    addr.sin_port = (u_short)htons(peer_port_);
    socklen_t addrlen = socklen_t(sizeof(addr));

    // connect
    int ret = ::connect(sockfd, (struct sockaddr*)&addr, addrlen);
    int savedErrno = (ret == 0) ? 0 : errno;

    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            close(sockfd);
            assert(false);
            break;
        default:
            close(sockfd);
            assert(false);
            break;
    }
}

void TcpClient::retry(int sockfd){
    close(sockfd);
    connect();
}
void TcpClient::connecting(int sockfd) {
    // add for polling writing event.
    printf("[TcpClient::%s] [connecting, waiting for handshake over, fd = %d]\n", __func__, sockfd);
    loop_->addFd(sockfd, EPOLLOUT | EPOLLERR,
            std::bind(&TcpClient::impossibleReadEvent, this, sockfd),
            std::bind(&TcpClient::newConnection, this, sockfd),
            std::bind(&TcpClient::impossibleErrorEvent, this, sockfd));
}

void TcpClient::disconnect()
{
    connection_->forceClose();
}

void TcpClient::newConnection(int sockfd)
{
    printf("[TcpClient::%s][connected, fd = %d]\n",__func__, sockfd);

    assert(connection_ == nullptr);

    struct sockaddr_in addr;
    memset(&addr, 0,sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof (addr));
    int ret = ::getsockname(sockfd, (struct sockaddr*)&addr, &addrlen);
    assert(ret >= 0);

    connection_ = new TcpConnection(loop_, sockfd, addr);
    connection_->setConnectionCallback(connectionCallback_);
    connection_->setMessageCallback(messageCallback_);
    connection_->setPkgDecodeCallback(pkgDecodeCallback_);
    //conn->setWriteCompleteCallback(writeCompleteCallback_);
    connection_->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));

    connectionCallback_(connection_);

    loop_->updateFd(sockfd, EPOLLIN | EPOLLERR | EPOLLHUP,
            std::bind(&TcpConnection::handleRead, connection_),
            std::bind(&TcpConnection::handleWrite, connection_),
            std::bind(&TcpConnection::handleError, connection_));
}

void TcpClient::removeConnection(const TcpConnection* conn)
{
    printf("[TcpClient::%s][disconnected, fd = %d]\n", __func__, conn->get_fd());
    if(conn) {
        loop_->removeFd(conn->get_fd());
        delete conn;
        conn = nullptr;
    }
    if (retry_){
        connect();
    }
}


void TcpClient::impossibleReadEvent(int sockfd){
    // assert(false);
    printf("[TcpClient::%s][fd=%d][impossibleReadEvent][errno=%d]\n", __func__, sockfd, errno);
    removeConnection(this->connection_);
}

void TcpClient::impossibleErrorEvent(int sockfd){
    // assert(false);
    printf("[TcpClient::%s][fd=%d][impossibleErrorEvent][errno=%d]\n", __func__, sockfd, errno);
    removeConnection(this->connection_);
}