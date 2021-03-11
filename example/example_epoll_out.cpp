//
// Created by jackyan on 2020/11/14.
//

#include <bits/stdc++.h>
#include "../common/TcpServer.h"
#include "../common/TcpClient.h"
using namespace std;

int decodeMessage(const char* msg, int len){
    if(len < 4){
        return 0;
    }
    auto pkgSize = ntohl(*(int32_t *) msg);
    if(pkgSize >= 10240 || pkgSize < 0){
        return -1;
    }
    return pkgSize;
}

std::string packMessage(int msgid, int uin){
    const int send_size = 1024  * 1024 * 5;
    static char buff[send_size];
    int body_len = send_size - 12;
    std::string msg(body_len, 'a');
    int msg_len = htonl(msg_len+12);
    msgid = htonl(msgid);
    uin = htonl(uin);
    *(uint32_t*)buff = msg_len;
    *(uint32_t*)(buff+4) = msgid;
    *(uint32_t*)(buff+8) = uin;
    memcpy(buff+12, msg.c_str(), body_len);
    return std::string(buff, body_len+12);
}
void onConnection(const TcpConnection* conn){
    // you can echo peer's ip and port here.
    if(conn->connected()){
        printf("[Main::connected] [fd = %d]\n",  conn->get_fd());
    }else{
        printf("[Main::disconnected] [fd = %d]\n",  conn->get_fd());
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len){
    printf("[Main::onMessage]  [pkgSize=%d]\n", len);
}

int main(){
    int listen_port = 8081;

    EventLoop loop;

    TcpServer server(&loop, listen_port);
    server.setConnectionCallback(std::bind(&onConnection, _1));
    server.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    server.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));
    server.start(1024);   // enable reading event

    TcpClient client1(&loop, "127.0.0.1", listen_port);
    TcpClient client2(&loop, "127.0.0.1", listen_port);
//    client1.setConnectionCallback(std::bind(&onConnection, _1));
//    client1.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
//    client1.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));
    client1.connect();
    client2.connect();
    client2.setConnectionCallback(std::bind(&onConnection, _1));
    client2.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    client2.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));

    do{
        loop.update(1);
    }while(client2.connection() == nullptr);

    client2.connection()->send(packMessage(1, 1));
    client2.connection()->send(packMessage(1, 1));
    std::cout << "[Main::send over]" << std::endl;

    do{
        loop.update(1);
    }while(true);


    return 0;
}