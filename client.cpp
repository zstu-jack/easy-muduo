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

const char* ip = "127.0.0.1";
const int listen_port = 8080;
const int loop_time_out_ms = 15;
bool quit = false;

int decodeMessage(const char* msg, int len){
    if(len < 4){
        return 0;
    }
    auto pkgSize = ntohl(*(int32_t *) msg);
    if(pkgSize >= 10240 || pkgSize < 0){ // TODO: drop the connection here.
        return -1;
    }
    return pkgSize;
}

void onConnection(const TcpConnection* conn)
{
    printf("connecting status = %d, fd = %d", (int)conn->connected(), conn->get_fd());
    if (conn->connected())
    {
        // connected
    }
    else
    {
        // disconnected
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len)
{
    // consume message ;
}

int main(int argc, char* argv[])
{
    EventLoop loop;

    TcpClient client(&loop, ip, listen_port);
    client.setConnectionCallback(std::bind(&onConnection,  _1));
    client.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    client.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2));
    //client.setWriteCompleteCallback(std::bind(&onWriteComplete, this, _1));=
    client.connect();

    do{
        loop.update(loop_time_out_ms);
    }while(!quit);

}

