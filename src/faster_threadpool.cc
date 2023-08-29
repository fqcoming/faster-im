
#include <iostream>
#include "faster_threadpool.h"



/* FasterThread */

FasterThread::FasterThread() : _isRunning(false) {
}


FasterThread::~FasterThread() {
}


void FasterThread::init(std::function<void(faster_event_t*)> onMessage) {
    _isRunning = true;
    _onMessage = onMessage;
    _thread.reset(new std::thread(std::bind(&FasterThread::threadFunc, this)));
}


void FasterThread::stop() {
    _isRunning = false;
    _condVar.notify_one();
    if (_thread->joinable()) {
        _thread->join();
    }
}


void FasterThread::addEvent(faster_event_t* event) {
    _eventQue.write(event, false);
    if (!_eventQue.flush()) {
        std::unique_lock<std::mutex> lock(_mutex);
        _condVar.notify_one();
    }
}



void FasterThread::threadFunc() {

    faster_event_t *event;
    while (true) {

		if (_eventQue.read(&event)) {
            _onMessage(event);
		} else {
			std::unique_lock<std::mutex> lock(_mutex);
			_condVar.wait(lock);
		}
        if (!_isRunning) break;
	}
}




/* FasterThreadPool */

FasterThreadPool::FasterThreadPool() 
        : _threadNum(0), _indexThread(-1) {
}


FasterThreadPool::~FasterThreadPool() {
}


void FasterThreadPool::init(int threadNum, std::function<void(faster_event_t*)> onMessage) {
    for (int i = 0; i < threadNum; i++) {
        FasterThread* th = new FasterThread();
        th->init(onMessage);
        _threads.emplace_back(th);
        _threadNum++;
    }
    _indexThread = 0;
}



void FasterThreadPool::stop() {
    for (int i = 0; i < _threadNum; i++) {
        _threads[i]->stop();
    }
}



void FasterThreadPool::addEvent(faster_event_t* event) {

    if (0 <= _indexThread && _indexThread < _threadNum) {

        std::shared_ptr<FasterThread> curThread = _threads[_indexThread];
        curThread->addEvent(event);
        _indexThread = (_indexThread + 1) % _threadNum;

    } else {
        std::cout << "_indexThread error." << std::endl;
    }
}


