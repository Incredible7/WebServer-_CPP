#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <assert.h>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool {
public:
<<<<<<< HEAD

    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()){
=======
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
>>>>>>> f9bd2f2 (1)
        assert(threadCount > 0);
        for(size_t i = 0; i < threadCount; ++i){
            std::thread([pool = pool_]{
                std::unique_lock<std::mutex> locker(pool->mtx);
                //重复执行线程池内任务，每次记得上锁
                while(1){
                    if(!pool->tasks.empty()){
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed) break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if(static_cast<bool>(pool_)){
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };

    //?为什么是shared_ptr
    std::shared_ptr<Pool> pool_;
};


#endif 