#include <iostream>
#include "common/EventLoop.h"
#include "common/TcpServer.h"


const int listen_port = 8080;
const int loop_time_out_ms = 15;
bool quit = false;

void onConnection(const TcpConnection* conn){
    // you can echo peer's ip and port here.
    printf("%s", __FUNCTION__);
    printf("connected = %d\n", conn->connected());
}

void onMessage(const TcpConnection* conn, const char* msg, int len){
    // data->peek.
    // conn->send(msg);
}

// <0 error
// =0 wait for remain package.
// >0 package size.
int decodeMessage(const char* msg, int len){

}

int main(){
    EventLoop loop;

    TcpServer server(&loop, listen_port);
    server.setConnectionCallback(std::bind(&onConnection, _1));
    server.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    server.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));
    server.start();   // enable reading event

    // main loop.
    do{
        loop.update(loop_time_out_ms);
    }while(!quit);

    return 0;
}