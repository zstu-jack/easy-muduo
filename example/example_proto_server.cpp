#include <iostream>
#include "../common/EventLoop.h"
#include "../common/TcpServer.h"
#include "protocal/test.pb.h"

const int listen_port = 8080;
const int loop_time_out_ms = 15;
bool quit = false;

std::map<TcpConnection*, int> conn_to_uid;
Logger logger(DETAIL, "server_log");

// == -1  error, which would lead to close the connection
// >  0   package size.
// == 0   wait for head.
int decodeMessage(const TcpConnection* conn, const char* msg, int len){
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
void onConnection(const TcpConnection* conn){
    // you can echo peer's ip and port here.
    if(conn->connected()){
        printf("[Main::connected] [fd = %d]\n",  conn->get_fd());
    }else{
        printf("[Main::disconnected] [fd = %d]\n",  conn->get_fd());
        // conn_to_uid
        conn_to_uid.erase((TcpConnection*)conn);
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len){
    auto ptr = msg;
    auto pkgSize = ntohl(*(uint32_t *) ptr);
    auto msgId = ntohl(*(uint32_t *) (ptr+4));
    auto uid = ntohl(*(uint32_t *) (ptr+8));

    if(conn_to_uid.find((TcpConnection*)conn) == conn_to_uid.end()){
        conn_to_uid[(TcpConnection*)conn] = uid;
    }

    logger.log(DETAIL,"[Main::onMessgae] [uid=%d] [msgId=%d] [pkgSize=%d]\n", uid, msgId, pkgSize);

    switch (msgId){
        case test::MSGID::REQ_PLAYER_MESSAGE:{
            test::ReqPlayerMessage req;
            req.ParseFromArray(ptr+12, len-12);
            logger.log(WARNING, "[Main::onMessage] [recv message] [uid=%d] [msg=%s] [connection's size=%d]\n", req.uid(), req.value().c_str(), conn_to_uid.size());

            test::InfPlayerMessage inf;
            inf.set_uid(req.uid());
            inf.set_value(req.value());

            for(auto iter = conn_to_uid.begin(); iter != conn_to_uid.end(); iter++){
                iter->first->send(packMessage(test::MSGID::INF_PLAYER_MESSAGE, iter->second, inf));
            }
        }
    }
}

int main(){
    EventLoop loop;
    loop.set_logger(&logger);

    TcpServer server(&loop, listen_port);
    server.setConnectionCallback(std::bind(&onConnection, _1));
    server.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    server.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2, _3));
    server.start(1024);   // enable reading event

    // main loop.
    do{
        loop.update(loop_time_out_ms);
    }while(!quit);

    return 0;
}