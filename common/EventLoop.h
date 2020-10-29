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
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

public:
    void update(int time_out);
    void push_functor(std::function<void()>);

private:
    void updateFd(int fd,  ReadEventCallback callback, int event = 0);
    void removeFd(int fd);
    void do_pending_functors();
private:
    int epoll_fd_;
    struct epoll_event* event_list_;
    std::map<int, ReadEventCallback> read_event_callback_;
    std::vector<std::function<void()>> pending_functors_;

};

#endif
