#ifndef DEFINE_H
#define DEFINE_H

class TcpConnection;
class RecvData;

typedef std::function<void()> ReadEventCallback;

typedef std::function<void (const TcpConnection*)> ConnectionCallback;
typedef std::function<void (const TcpConnection*)> CloseCallback;
typedef std::function<void (const TcpConnection*, const char* msg, int len)> MessageCallback;
typedef std::function<int (const char*, int)> PkgDecodeCallback;

using namespace std::placeholders;

#endif //DEFINE_H
