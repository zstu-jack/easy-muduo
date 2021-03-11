#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <functional>
#include <map>
#include <sys/epoll.h>

#include "Log.h"
#include "Define.h"

class EventCallbacks{
public:
    ReadEventCallback read_event_callback_;
    WriteEventCallback write_event_callback_;
    ErrorEventCallback error_event_callback_;
    EventCallbacks(ReadEventCallback& rcb, WriteEventCallback& wcb, ErrorEventCallback& ecb);
    EventCallbacks(const EventCallbacks& callbacks);
    EventCallbacks& operator=(const EventCallbacks&);
    EventCallbacks();
};

class EventLoop{
public:
    friend class TcpServer;
    friend class TcpClient;
    friend class TcpConnection;
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

public:
    void update(int time_out);
    void push_functor(std::function<void()>);
    void set_logger(Logger* logger);

private:
    void addFd(int fd, int event,  ReadEventCallback callback, WriteEventCallback writeEventCallback, ErrorEventCallback errorEventCallback);
    void updateFd(int fd, int event,  ReadEventCallback callback, WriteEventCallback writeEventCallback, ErrorEventCallback errorEventCallback);
    void removeFd(int fd);
    void do_pending_functors();
    void log(int32_t log_level, const char* buffer, ...) __attribute__((format(printf, 3, 4)));
private:
    int epoll_fd_;
    struct epoll_event* event_list_;
    std::map<int, EventCallbacks> event_callbacks_;
    std::vector<std::function<void()>> pending_functors_;
    Logger* logger;

};

#endif
