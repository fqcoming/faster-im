#ifndef __FASTER_THREAD_POOL_H__
#define __FASTER_THREAD_POOL_H__

#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "ypipe.hpp"
#include "fasterconn.h"


#define QUEUE_CHUNK_SIZE    100



class FasterThread final
{
public:
    FasterThread();
    ~FasterThread();

    void init();
    void stop();

    FasterThread(const FasterThread& rhs) = delete;
    FasterThread& operator=(const FasterThread& rhs) = delete;

    void setDoMessageCallback(std::function<void()>);

private:
    void threadFunc();

private:
    ypipe_t<faster_event_t*, QUEUE_CHUNK_SIZE> _eventQue;
    std::shared_ptr<std::thread> _thread;
    std::mutex _mutex;
    std::condition_variable _condVar;
    bool isRunning;
    std::function<void()> doMessage;
};







class FasterThreadPool final
{
public:
    FasterThreadPool();
    ~FasterThreadPool();

    void init(int threadNum = 4);
    void stop();

    void addEvent();

    FasterThreadPool(const FasterThreadPool& rhs) = delete;
    FasterThreadPool& operator=(const FasterThreadPool& rhs) = delete;

private:
    std::vector<FasterThread> _threads;
};






#endif