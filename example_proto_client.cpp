#include <utility>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <arpa/inet.h>

#include "common/TcpConnection.h"
#include "common/TcpClient.h"
#include "common/EventLoop.h"

#include "protocal/test.pb.h"

const char* ip = "127.0.0.1";
const int listen_port = 8080;
const int loop_time_out_ms = 15;
bool quit = false;

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
std::string packMessage(int msgid, int uin, google::protobuf::Message& message){
    static char buff[10240];
    std::string body = message.SerializeAsString();
    int msg_len = body.size() + 12;
    assert(msg_len <= 10240);

    msg_len = htonl(msg_len);
    msgid = htonl(msgid);
    uin = htonl(uin);

    *(uint32_t*)buff = msg_len;
    *(uint32_t*)(buff+4) = msgid;
    *(uint32_t*)(buff+8) = uin;
    return std::string(buff, 12) + body;
}

void onConnection(const TcpConnection* conn)
{
    if (conn->connected())
    {
        printf("[Main::connected] [fd = %d]\n",  conn->get_fd());
        // connected. do something here
    }
    else
    {
        printf("[Main::disconnected] [fd = %d]\n", conn->get_fd());
        // disconnected. do something here
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len)
{
    // consume message ;
    auto ptr = msg;
    auto pkgSize = ntohl(*(uint32_t *) ptr);
    auto msgId = ntohl(*(uint32_t *) (ptr+4));
    auto uid = ntohl(*(uint32_t *) (ptr+8));

    printf("[Main::%s] [fd = %d] [uin=%d] [msgId=%d] [pkgSize=%d]\n",__func__, conn->get_fd(), uid, msgId, pkgSize);

    switch (msgId){
        case test::MSGID::INF_PLAYER_MESSAGE:{
            test::ReqPlayerMessage req;
            req.ParseFromArray(ptr+12, len-12);
            printf("[Main::%s] [uid=%d] [msg=%s]\n", __func__, req.uid(), req.value().c_str());
        }
    }
}

int main(int argc, char* argv[])
{
    EventLoop loop;

    //1. connecting
    TcpClient client(&loop, ip, listen_port);
    client.setConnectionCallback(std::bind(&onConnection,  _1));
    client.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    client.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));
    //client.setWriteCompleteCallback(std::bind(&onWriteComplete, this, _1));
    client.connect();
    do{
        loop.update(loop_time_out_ms);
    }while(!client.connected());

    //2. send broadcast msg
    test::ReqPlayerMessage req1;
    req1.set_uid(1);
    req1.set_value("message1");
    client.connection()->send(packMessage(test::MSGID::REQ_PLAYER_MESSAGE, 1, req1));
    for(int i = 0; i < 10; ++ i){
        loop.update(loop_time_out_ms);
    }

    //3. another connection
    TcpClient client2(&loop, ip, listen_port);
    client2.setConnectionCallback(std::bind(&onConnection,  _1));
    client2.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    client2.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));
    //client.setWriteCompleteCallback(std::bind(&onWriteComplete, this, _1));=
    client2.connect();
    do{
        loop.update(loop_time_out_ms);
    }while(!client2.connected());


    //4. another connection send broadcast message.
    test::ReqPlayerMessage req2;
    req2.set_uid(1);
    req2.set_value("message2");
    client2.connection()->send(packMessage(test::MSGID::REQ_PLAYER_MESSAGE, 2, req2));

    // loop forever;
    do{
        loop.update(loop_time_out_ms);
    }while(!quit);

}

