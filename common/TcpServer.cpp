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

void TcpServer::start(int backlog){
    socket->listen(backlog);
    loop_->addFd(socket->fd(), EPOLLIN | EPOLLERR | EPOLLHUP,
            std::bind(&TcpServer::newConnection, this),
            std::bind(&TcpServer::impossibleWriteEvent, this),
            std::bind(&TcpServer::impossibleErrorEvent, this));
}

void TcpServer::newConnection(){

    struct sockaddr_in peer_addr_;
    int conn_fd = this->socket->accept(&peer_addr_);

    loop_->log(DETAIL,"conn = %s, fd = %d\n", __func__, conn_fd);

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

    if(connectionCallback_){
        connectionCallback_(tcpConnection);
    } else{
        loop_->log(DETAIL,"[new client connected but user-space connection callback is not set, conn_fd = %d]\n", conn_fd);
    }
}
void TcpServer::removeConnection(const TcpConnection* conn){
    int fd = conn->get_fd();
    loop_->log(DETAIL,"removeConnection fd = %d\n", fd);
    loop_->removeFd(fd);
    delete connectionMap_[fd];
    connectionMap_.erase(fd);
}

void TcpServer::impossibleWriteEvent(){
    loop_->log(ERROR,"[fd=%d] something unexpected happened...errno=%d\n" ,socket->fd(), errno);
}
void TcpServer::impossibleErrorEvent(){
    loop_->log(ERROR,"[fd=%d] something unexpected happened...errno=%d\n" ,socket->fd(), errno);
}