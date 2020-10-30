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
void TcpConnection::send(const void* message, int len){

    char* message_ptr = (char*)message;
    printf("fd = %d, state = %d, send len = %d\n", get_fd(), state_, len);
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
    int n = read(socket_->fd(), read_buf + read_buf_index, socket_buff_size - read_buf_index);

    printf("fd = %d read %d bytes\n", socket_->fd(), n);

    if (n > 0){
        int pkg_index = 0;
        read_buf_index += n;
        for(;;){
            int ret = pkgDecodeCallback_(read_buf + pkg_index, read_buf_index - pkg_index);
            printf("pkgsize=%d, decode ret = %d\n", read_buf_index - pkg_index,  ret);
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

    }
}

void TcpConnection::handleWrite(){


}

void TcpConnection::handleClose() {

    printf("fd = %d closed\n", get_fd());

    state_ = kDisconnected;
    connectionCallback_(this);
    loop_->push_functor(std::bind(closeCallback_, this));
}

void TcpConnection::handleError(){

}

int TcpConnection::get_fd()const{
    return socket_->fd();
}


