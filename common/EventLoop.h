#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <functional>
#include <map>
#include <sys/epoll.h>

#include "Define.h"

class EventLoop{
public:
    friend class TcpServer;
    friend class TcpClient;
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

public:
    void update(int time_out);
    void push_functor(std::function<void()>);

private:
    void addFd(int fd, int event,  ReadEventCallback callback, WriteEventCallback writeEventCallback, ErrorEventCallback errorEventCallback);
    void updateFd(int fd, int event,  ReadEventCallback callback, WriteEventCallback writeEventCallback, ErrorEventCallback errorEventCallback);
    void removeFd(int fd);

    void do_pending_functors();
private:
    int epoll_fd_;
    struct epoll_event* event_list_;
    std::map<int, ReadEventCallback> read_event_callback_;
    std::map<int, WriteEventCallback> write_event_callback_;
    std::map<int, ErrorEventCallback> error_event_callback_;
    std::vector<std::function<void()>> pending_functors_;

};

#endif
