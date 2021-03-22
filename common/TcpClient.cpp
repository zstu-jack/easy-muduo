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
    state_ = kDisconnected;
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
    if (state_ == kConnecting || state_ == kConnected){
        return;
    }

    connect_ = 1;

    // client fd.
    sockfd = createNonBlockSocket();
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
    // FIXME: retry next tick or retry with a delay.
    close(sockfd);
    connect();
}
void TcpClient::connecting(int sockfd) {
    // add for polling writing event.
    state_ = kConnecting;
    loop_->EASY_LOG(DETAIL, "[connecting, waiting for handshake over, fd = %d]", sockfd);
    loop_->addFd(sockfd, EPOLLOUT | EPOLLERR,
            std::bind(&TcpClient::impossibleReadEvent, this, sockfd),
            std::bind(&TcpClient::newConnection, this, sockfd),
            std::bind(&TcpClient::impossibleErrorEvent, this, sockfd));
}

void TcpClient::disconnect()
{
    connect_ = 0;
    if(this->connection_) {
        connection_->forceClose();
    }
}
bool TcpClient::connected(){
    return connection_ != nullptr && this->connection_->connected();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->EASY_LOG(DETAIL, "[fd = %d]", sockfd);
    if (connection_ != nullptr){
        loop_->EASY_LOG(WARNING, "already connected fd = %d", sockfd);
        return;
    }
    state_ = kConnected;

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

    if(connectionCallback_ != nullptr) {
        connectionCallback_(connection_);
    }else{
        loop_->EASY_LOG(DETAIL, "[connected but user-space connection callback is not set, fd = %d]", sockfd);
    }

    loop_->updateFd(sockfd, EPOLLIN | EPOLLERR | EPOLLHUP,
            std::bind(&TcpConnection::handleRead, connection_),
            std::bind(&TcpConnection::handleWrite, connection_),
            std::bind(&TcpConnection::handleError, connection_));
}

void TcpClient::removeConnection(const TcpConnection* conn)
{

    // FIXME: for client, ignore the argument.
    state_ = kDisconnected;
    if(this->connection_) {
        loop_->EASY_LOG(DETAIL, "[disconnected, fd = %d]", this->connection_->get_fd());
        delete this->connection_;
        this->connection_ = nullptr;
    }
    if (retry_ && connect_){
        connect();
    }
}


void TcpClient::impossibleReadEvent(int sockfd){
    // assert(false);
    loop_->EASY_LOG(FATAL, "[fd=%d][impossibleReadEvent][errno=%d]", sockfd, errno);
    close(sockfd);
    removeConnection(this->connection_);
}

void TcpClient::impossibleErrorEvent(int sockfd){
    // assert(false);
    loop_->EASY_LOG(FATAL, "[fd=%d][impossibleErrorEvent][errno=%d]", sockfd, errno);
    close(sockfd);
    removeConnection(this->connection_);
}