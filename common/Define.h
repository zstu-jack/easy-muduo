#ifndef DEFINE_H
#define DEFINE_H

#include <functional>

class TcpConnection;

typedef std::function<void()> ReadEventCallback;
typedef std::function<void()> WriteEventCallback;
typedef std::function<void()> ErrorEventCallback;

typedef std::function<void (const TcpConnection*)> ConnectionCallback;
typedef std::function<void (const TcpConnection*)> CloseCallback;
typedef std::function<void (const TcpConnection*, const char* msg, int len)> MessageCallback;

// <  0: error occurs, close connection.
// == 0: not enough data for packing a package.
// >  0: package size
typedef std::function<int (const char*, int)> PkgDecodeCallback;

using namespace std::placeholders;

#endif //DEFINE_H
