#include <cassert>

#include "TcpServer.h"
#include "EventLoop.h"
#include "Socket.h"

TcpServer::TcpServer(EventLoop* loop, int listen_port){
    this->loop_ = loop;
    this->listen_port_ = listen_port;
    this->socket = new Socket(createNonBlockSocket());
    this->socket->bindAddress(listen_port);
}

TcpServer::~TcpServer(){
    delete socket;
    for(auto conn : connectionMap_){
        assert(conn.second);
        delete conn.second;
        conn.second = nullptr;
    }
    connectionMap_.clear();
}

void TcpServer::start(){
    socket->listen(1024);
    loop_->addFd(socket->fd(), EPOLLIN | EPOLLERR | EPOLLHUP,
            std::bind(&TcpServer::newConnection, this),
            std::bind(&TcpServer::impossibleEvent, this),
            std::bind(&TcpServer::impossibleEvent, this));          // TODO: replace impossibleEvent with proper callback.
}

void TcpServer::newConnection(){

    struct sockaddr_in peer_addr_;
    int conn_fd = this->socket->accept(&peer_addr_);

    printf("[TcpServer::%s] fd = %d\n", __func__, conn_fd);

    TcpConnection* tcpConnection = new TcpConnection(loop_, conn_fd, peer_addr_);
    connectionMap_[tcpConnection->get_fd()] = tcpConnection;

    // user callback
    tcpConnection->setConnectionCallback(connectionCallback_);
    tcpConnection->setMessageCallback(messageCallback_);
    tcpConnection->setPkgDecodeCallback(pkgDecodeCallback_);
    tcpConnection->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    // conn->setWriteCompleteCallback(writeCompleteCallback_);

    // add for polling.
    loop_->addFd(tcpConnection->get_fd(), EPOLLIN | EPOLLERR | EPOLLHUP,
            std::bind(&TcpConnection::handleRead, tcpConnection),
            std::bind(&TcpConnection::handleWrite, tcpConnection),
            std::bind(&TcpConnection::handleError, tcpConnection));
    connectionCallback_(tcpConnection);
}
void TcpServer::removeConnection(const TcpConnection* conn){
    printf("[TcpServer::%s] fd = %d\n", __func__, conn->get_fd());

    int fd = conn->get_fd();
    loop_->removeFd(fd);
    delete connectionMap_[fd];
    connectionMap_.erase(fd);
}

void TcpServer::impossibleEvent(){
    printf("[fd=%d] something unexpected happened...errno=%d\n", socket->fd(), errno);
}