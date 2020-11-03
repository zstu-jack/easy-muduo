#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>

class Socket
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd){}
    ~Socket();
    int fd() const { return sockfd_; }
    void bindAddress(struct sockaddr_in* addr_);
    void bindAddress(int listen_port);
    void listen(int backlog);
    int accept(struct sockaddr_in* addr_);
    void shutdownWrite();
    void setTcpNoDelay(bool on); //TCP_NODELAY
    void setReuseAddr(bool on);  //SO_REUSEADDR
    void setReusePort(bool on);  //SO_REUSEPORT
    void setKeepAlive(bool on);  //SO_KEEPALIVE

private:
    const int sockfd_;
};

int createNonBlockSocket();

#endif  // SOCKET_H
