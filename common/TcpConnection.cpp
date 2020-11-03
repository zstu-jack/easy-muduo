#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include "TcpConnection.h"

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, struct sockaddr_in& peerAddr){
    this->loop_ = loop;
    this->socket_ = new Socket(sockfd);
    this->peerAddr_ = peerAddr;
    this->state_ = kConnected;

    this->socket_->setKeepAlive(true);
    this->socket_->setReuseAddr(true);
    this->socket_->setReusePort(true);
}
TcpConnection::~TcpConnection(){
    delete socket_;
}
void TcpConnection::send(const char* message, int len){
    printf("[TcpConnection::%s] [fd=%d, state=%d, send_len=%d]\n", __func__, get_fd(), state_, len);
    if(len == 0){
        return ;
    }

    int sended_len = 0;
    if (state_ == kConnected){
        while (len > 0)
        {
            int sended_byte = ::send(socket_->fd(), message + sended_len, (size_t)len - sended_len, 0);
            sended_len += sended_byte;
            if (sended_byte > 0){ // may be interrupted when sending. so we send data in loop.
                sended_len  += sended_byte;
            }else{  // error occur or peer's buffer is full.
                if(EAGAIN != errno){ // error occurs.
                    printf("[%s][errno=%d]\n", __FUNCTION__, errno);
                    handleClose();
                    break;
                }else{ // peer's tcp buffer is full.
                    printf("[TcpConnection::%s][peer is full, waiting for writing(fd=%d)]\n", __func__, get_fd());
                    int write_buffer_size = socket_buff_size - write_buf_size;
                    if(write_buffer_size < len - sended_len){ // write buffer is full.
                        printf("[TcpConnection::%s][write buffer is full][force close here(fd=%d)]\n", __func__, get_fd());
                        handleClose();
                    }else{
                        memcpy(write_buf+write_buf_size, message+sended_len, len - sended_len);
                        loop_->updateFd(get_fd(), EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLHUP,
                                        std::bind(&TcpConnection::handleRead, this),
                                        std::bind(&TcpConnection::handleWrite, this),
                                        std::bind(&TcpConnection::handleError, this));
                    }
                }
                break;
            }
        }
        return ;
    }
    printf("[TcpConnection::%s][not connected]\n", __func__);
}
void TcpConnection::send(const std::string& message){
    send(message.c_str(), message.length());
}

void TcpConnection::forceClose(){
    handleClose();
}

void TcpConnection::handleRead(){
    int n = read(socket_->fd(), read_buf + read_buf_index, socket_buff_size - read_buf_index); // TODO: remove EPOLL_IN when buffer is full.

    printf("[TcpConnection::%s][fd = %d read %d bytes]\n", __func__, get_fd(), n);

    if (n > 0){
        int pkg_index = 0;
        read_buf_index += n;
        for(;;){
            int ret = pkgDecodeCallback_(read_buf + pkg_index, read_buf_index - pkg_index);
            if(ret < 0){ // error
               handleClose();
               break;
            }else if(ret == 0 || ret > read_buf_index - pkg_index){ // waiting for a full pkg.
                break;
            }else{
                messageCallback_(this, read_buf + pkg_index, ret);
                pkg_index += ret;
            }
        }
        if(pkg_index != 0){
            int size = read_buf_index - pkg_index;
            std::memmove(read_buf, read_buf + pkg_index, size);
            read_buf_index = size;
        }
    }
    else if (n == 0){
        handleClose();
    }else{
        if(errno != EAGAIN){
            printf("[TcpConnection::%s][errno=%d]\n", __FUNCTION__, errno);
            handleClose();
        }
    }
}

void TcpConnection::handleWrite(){
    int sended_len = 0;
    while(sended_len < write_buf_size){
        int sended_byte = ::send(socket_->fd(), write_buf, write_buf_size, 0);
        if(sended_byte > 0){
            sended_len += sended_byte;
        }else {
            if(EAGAIN != errno){ // error occurs.
                printf("[TcpConnection::%s][errno=%d]\n", __FUNCTION__, errno);
                handleClose();
            }else{ // peer's tcp buffer is full.
                if(sended_len) {
                    memmove(write_buf, write_buf + sended_len, write_buf_size - sended_len);
                    write_buf_size = write_buf_size - sended_len;
                }
            }
            return ;
        }
    }
    // remove EPOLL_OUT event.
    loop_->updateFd(get_fd(), EPOLLIN | EPOLLERR | EPOLLHUP,
                    std::bind(&TcpConnection::handleRead, this),
                    std::bind(&TcpConnection::handleWrite, this),
                    std::bind(&TcpConnection::handleError, this));
}

void TcpConnection::handleClose() {
    printf("[TcpConnection::%s] fd = %d closed\n", __func__, get_fd());
    state_ = kDisconnected;
    connectionCallback_(this);
    loop_->push_functor(std::bind(closeCallback_, this));
}

void TcpConnection::handleError(){
    printf("[TcpConnection::%s][fd=%d]\n", __func__, get_fd());
    handleClose();
}

int TcpConnection::get_fd()const{
    return socket_->fd();
}


