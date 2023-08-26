

#include "fasterthreadpool.h"



/* FasterThread */

FasterThread::FasterThread() : isRunning(false) {
}


FasterThread::~FasterThread() {
}


void FasterThread::init() {
    isRunning = true;
    _thread.reset(new std::thread(std::bind(&FasterThread::threadFunc, this)));
}


void FasterThread::stop() {
    isRunning = false;
    _condVar.notify_one();
    if (_thread->joinable()) {
        _thread->join();
    }
}


void FasterThread::threadFunc() {

    faster_event_t *event;
    while (true) {

		if (_eventQue.read(&event)) {
            doMessage();
		} else {
			std::unique_lock<std::mutex> lock(_mutex);
			_condVar.wait(lock);
		}

        if (!isRunning) break;
	}

}


/* FasterThreadPool */

FasterThreadPool::FasterThreadPool() {

}


FasterThreadPool::~FasterThreadPool() {

}


void FasterThreadPool::init(int threadNum) {

}



void FasterThreadPool::stop() {

}



void FasterThreadPool::addEvent() {
    
}



