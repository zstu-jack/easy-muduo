#ifndef DEFINE_H
#define DEFINE_H

#include <functional>
#include <string.h>

class TcpConnection;

typedef std::function<void()> ReadEventCallback;
typedef std::function<void()> WriteEventCallback;
typedef std::function<void()> ErrorEventCallback;

typedef std::function<void (const TcpConnection*)> ConnectionCallback;
typedef std::function<void (const TcpConnection*)> CloseCallback;
typedef std::function<void (const TcpConnection*)> HighWaterCallback;
typedef std::function<void (const TcpConnection*, const char* msg, int len)> MessageCallback;

// <  0: error occurs, close connection.
// == 0: not enough data for packing a package.
// >  0: package size
typedef std::function<int (const TcpConnection*, const char*, int)> PkgDecodeCallback;

using namespace std::placeholders;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define EASY_LOG(LEVEL, X, ...) log(LEVEL, "[%20s:%-3d, %20s()] [" X "]\n" , __FILENAME__,  __LINE__,__FUNCTION__, ##__VA_ARGS__)

enum StateE { kDisconnected, kConnected, kConnecting };

#endif //DEFINE_H
