#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <fcntl.h>
#include "Socket.h"
#include "EventLoop.h"

Socket::~Socket(){
    close(sockfd_);
}

void Socket::bindAddress(struct sockaddr_in* addr)
{
    int ret = ::bind(sockfd_, (const sockaddr*)addr, socklen_t(sizeof(addr)));
    assert(ret >= 0);
}

void Socket::bindAddress(int listen_port){
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = (u_short)htons(listen_port);
    socklen_t addrlen = socklen_t(sizeof(addr));
    int ret = ::bind(fd(), (const sockaddr*) &addr, addrlen);
    assert(ret == 0);
}

void Socket::listen(int backlog)
{
    int ret = ::listen(sockfd_, backlog);
    assert(ret >= 0);
}

int Socket::accept(struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int sockfd = ::accept(fd(), (struct sockaddr*)addr, &addrlen);

    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);

    assert(sockfd >= 0);

    return sockfd;
}


void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,&optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,&optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    assert(ret >= 0 && on || (!on));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,&optval, static_cast<socklen_t>(sizeof optval));
}

int createNonBlockSocket(){
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0){
        printf("err = %s\n", strerror(errno));
    }
    assert(sockfd >= 0);
    return sockfd;
}



