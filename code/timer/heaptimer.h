<<<<<<< HEAD
//基于小根堆实现的定时器，关闭超时的非活动连接
=======
 //基于小根堆实现的定时器，关闭超时的非活动连接
>>>>>>> f9bd2f2 (1)
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;   //高精度时钟
typedef std::chrono::milliseconds MS;               //毫秒
typedef Clock::time_point TimeStamp;                //时间戳

struct TimerNode {
    int id;                 //标记定时器
    TimeStamp expires;      //设置过期时间
    TimeoutCallBack cb;     //设置一个回调函数方便删除定时器的同时将其对应的HTTP连接关闭

    bool operator<(const TimerNode& t) { return expires < t.expires; }
};

class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }

    //调整指定id的结点 
    void adjust(int id, int newExpires);
    //添加定时器 
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    //删除指定id结点，并触发回调函数
    void doWork(int id);
    int GetNextTick();
    //清除超时结点
    void tick();

    void pop();
    void clear();

private:
    std::vector<TimerNode> heap_;
    //映射一个fd对应的定时器在heap_中的位置
    std::unordered_map<int, size_t> ref_;

    //删除指定定时器，删除指定位置的结点
    void del_(size_t i);
    //向上调整
    void siftup_(size_t i);
    //向下调整
    bool siftdown_(size_t index, size_t n);
    //交换两个结点位置
    void SwapNode_(size_t i, size_t j);
};


#endif