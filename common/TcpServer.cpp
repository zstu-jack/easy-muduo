#include <cassert>

#include "TcpServer.h"
#include "EventLoop.h"
#include "Socket.h"

TcpServer::TcpServer(EventLoop* loop, int listen_port){
    this->loop_ = loop;
    this->listen_port_ = listen_port;
    this->socket = new Socket(createNonBlockSocket());

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = (u_short)htons(listen_port);
    socklen_t addrlen = socklen_t(sizeof(addr));
    int ret = ::bind(this->socket->fd(), (const sockaddr*) &addr, addrlen);         // TODO: move to Socket.
    assert(ret == 0);

}
TcpServer::~TcpServer(){
    delete socket;
    // TODO: delete TcpConnection In connectionMap_
}


void TcpServer::start(){
    int ret = ::listen(socket->fd(), 1024);                        // TODO: listen belongs to Socket. should be called Socket (etc: Socket::listen)
    assert(ret >= 0);
    this->loop_->addFd(socket->fd(), EPOLLIN | EPOLLERR | EPOLLHUP,
            std::bind(&TcpServer::newConnection, this),
            std::bind(&TcpServer::impossibleEvent, this),
            std::bind(&TcpServer::impossibleEvent, this));          // TODO: replace impossibleEvent with proper callback.
}

void TcpServer::newConnection(){

    struct sockaddr_in peer_addr_;
    int conn_fd = this->socket->accept(&peer_addr_);

    printf("new client connected, fd = %d\n", conn_fd);

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
    printf("removeConnection fd = %d\n", conn->get_fd());

    int fd = conn->get_fd();
    loop_->removeFd(fd);
    delete connectionMap_[fd];
    connectionMap_.erase(fd);
}

void TcpServer::impossibleEvent(){
    printf("something unexpected happened...\n");
}