#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>  //epoll_ctl()
#include <fcntl.h>      //fcntl()
#include <unistd.h>     //close()
#include <assert.h>     //assert()
#include <vector>
#include <errno.h>


class Epoller{
public:
    explicit Epoller(int maxEvent = 1024);  //禁止隐式转换和复制拷贝
    ~Epoller();

    bool AddFd(int fd, uint32_t events);    //添加epoll文件描述符

    bool ModFd(int fd, uint32_t events);    //操作epoll文件

    bool DelFd(int fd);                     //删除

    int Wait(int timeoutMS = -1);           //等待事件

    int GetEventFd(size_t i) const;         //获取事件描述符，禁止修改

    uint32_t GetEvents(size_t i) const;     //获取事件

private:
    int epollFd_;
    std::vector<struct epoll_event> events_; //事件数组
};


#endif