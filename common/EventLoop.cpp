#include "EventLoop.h"
#include "TcpServer.h"

#include <vector>

const int max_event_size = 1024;

EventCallbacks::EventCallbacks(){}
EventCallbacks::EventCallbacks(ReadEventCallback& rcb, WriteEventCallback& wcb, ErrorEventCallback& ecb){
    this->read_event_callback_ = rcb;
    this->write_event_callback_ = wcb;
    this->error_event_callback_ = ecb;
}
EventCallbacks::EventCallbacks(const EventCallbacks& callbacks){
    this->read_event_callback_ = callbacks.read_event_callback_;
    this->write_event_callback_ = callbacks.write_event_callback_;
    this->error_event_callback_ = callbacks.error_event_callback_;
}
EventCallbacks& EventCallbacks::operator=(const EventCallbacks& callbacks){
    this->read_event_callback_ = callbacks.read_event_callback_;
    this->write_event_callback_ = callbacks.write_event_callback_;
    this->error_event_callback_ = callbacks.error_event_callback_;
}

EventLoop::EventLoop(){
    epoll_fd_ = epoll_create(max_event_size);
    assert(epoll_fd_ >= 0);
    event_list_ = (struct epoll_event *) malloc(max_event_size * sizeof(struct epoll_event));
}
EventLoop::~EventLoop(){

}


void EventLoop::update(int timeout){
    do_pending_functors();

    int event_count = ::epoll_wait(epoll_fd_, event_list_, max_event_size, timeout);
    for(int32_t i = 0; i < event_count; i++) {
        struct epoll_event *one_event = event_list_+i;
        int32_t event_fd = one_event->data.fd;
        assert(event_callbacks_.count(event_fd));
        if (EPOLLERR & one_event->events){
            int error_code = 0;
            socklen_t len = (socklen_t) sizeof(error_code);
            getsockopt(event_fd, SOL_SOCKET, SO_ERROR, &error_code, &len);
            printf("[EventLoop::%s][fd=%d][error_code=%d]\n", __func__, event_fd, error_code);
            event_callbacks_[event_fd].error_event_callback_();
        }else if (EPOLLIN & one_event->events){
            event_callbacks_[event_fd].read_event_callback_();
        }else if (EPOLLOUT & one_event->events){
            event_callbacks_[event_fd].write_event_callback_();
        }
    }

    do_pending_functors();
}
void EventLoop::do_pending_functors(){
    for(size_t i = 0; i < pending_functors_.size(); ++ i){
        pending_functors_[i]();
    }
    pending_functors_.clear();
}

void EventLoop::push_functor(std::function<void()> func){
    this->pending_functors_.push_back(func);
}

void EventLoop::addFd(int fd, int event, ReadEventCallback callback, WriteEventCallback writeEventCallback, ErrorEventCallback errorEventCallback){
    assert(fd >= 0);

    struct epoll_event epoll_event;
    epoll_event.events = event;
    epoll_event.data.ptr = NULL;
    epoll_event.data.fd = fd;

    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &epoll_event);
    if(ret < 0){
        printf("[EventLoop::%s][errno=%d]\n", __func__, errno);
    }
    event_callbacks_.erase(fd);
    event_callbacks_.insert(std::make_pair(fd, EventCallbacks(callback, writeEventCallback, errorEventCallback)));
}

void EventLoop::updateFd(int fd,int events, ReadEventCallback callback, WriteEventCallback writeEventCallback, ErrorEventCallback errorEventCallback) {
    assert(fd >= 0);

    struct epoll_event epoll_event;
    epoll_event.events = events;
    epoll_event.data.ptr = NULL;
    epoll_event.data.fd = fd;

    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &epoll_event);
    if(ret < 0) {
        printf("[EventLoop::%s][errno=%d]\n", __func__, errno);
    }
    event_callbacks_.erase(fd);
    event_callbacks_.insert(std::make_pair(fd, EventCallbacks(callback, writeEventCallback, errorEventCallback)));
}

void EventLoop::removeFd(int fd) {
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    if(ret < 0) {
        printf("[EventLoop::%s][errno=%d]\n", __func__, errno);
    }
    event_callbacks_.erase(fd);
}

