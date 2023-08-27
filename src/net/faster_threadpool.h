#ifndef __FASTER_THREAD_POOL_H__
#define __FASTER_THREAD_POOL_H__

#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "ypipe.hpp"
#include "faster_eventpool.h"
#include "faster_connpool.h"


#define QUEUE_CHUNK_SIZE    100



class FasterThread final
{
public:
    FasterThread();
    ~FasterThread();

    void init();
    void stop();
    void addEvent(faster_event_t* event);

    FasterThread(const FasterThread& rhs) = delete;
    FasterThread& operator=(const FasterThread& rhs) = delete;

    void setOnMessageCallback(std::function<void(faster_event_t*)> onMessage);

private:
    void threadFunc();

private:
    ypipe_t<faster_event_t*, QUEUE_CHUNK_SIZE> _eventQue;
    std::shared_ptr<std::thread> _thread;
    std::mutex _mutex;
    std::condition_variable _condVar;
    bool _isRunning;
    std::function<void(faster_event_t*)> _onMessage;
};







class FasterThreadPool final
{
public:
    FasterThreadPool();
    ~FasterThreadPool();

    void init(int threadNum = 4);
    void stop();

    void addEvent(faster_event_t* event);

    FasterThreadPool(const FasterThreadPool& rhs) = delete;
    FasterThreadPool& operator=(const FasterThreadPool& rhs) = delete;

private:
    std::vector<FasterThread*> _threads;
    int _threadNum;
    int _indexThread;      // Identify which thread to add events to currently.
};






#endif