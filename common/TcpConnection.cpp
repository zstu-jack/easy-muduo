#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "TcpConnection.h"

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, struct sockaddr_in& peerAddr){
    this->loop_ = loop;
    this->socket_ = new Socket(sockfd);
    this->peerAddr_ = peerAddr;
    this->socket_->setKeepAlive(true);
    this->state_ = kConnected; // TODO: set flag later (once we can write)
}
TcpConnection::~TcpConnection(){
    delete socket_;
}
void TcpConnection::send(const void* message, int len){
    char* message_ptr = (char*)message;
    if (state_ == kConnected){
        while (len > 0)
        {
            int sended_byte = ::send(socket_->fd(), message_ptr, (size_t)len, 0);
            if (sended_byte > 0){
                message_ptr += sended_byte;
                len -= sended_byte;
            }
            if (sended_byte < 0 && EAGAIN != errno)
            {
                printf("[%s][errno=%d]\n",__FUNCTION__, errno);
                handleClose();
                break;
            }
        }
    }
}
void TcpConnection::send(const std::string& message){
    send(message.c_str(), message.length());

}
void TcpConnection::forceClose(){
    handleClose();
}

void TcpConnection::handleRead(){
    int n = read(socket_->fd(), read_buf, socket_buff_size - read_buf_index);
    if (n > 0){
        read_buf_index += n;
        // TODO
//        for(;;){
//
//
//        }
    }
    else if (n == 0){
        handleClose();
    }else{

    }
}
void TcpConnection::handleClose() {

    printf("fd = %d closed\n", get_fd());

    state_ = kDisconnected;
    connectionCallback_(this);
    loop_->push_functor(std::bind(closeCallback_, this));
}


int TcpConnection::get_fd()const{
    return socket_->fd();
}


