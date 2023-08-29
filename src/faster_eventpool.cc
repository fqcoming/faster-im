
#include <iostream>
#include <string.h>
#include "faster_eventpool.h"




FasterEventPool::FasterEventPool() : 
    _freeEventSize(0), _usedEventSize(0), _eventBufCapacity(0) {
}


FasterEventPool::~FasterEventPool() {

    for (int i = 0; i < _freeEventSize; i++) {
        faster_event_t* event = _freeEvents.front();
        _freeEvents.pop_front();
        delete event;
    }
    _freeEventSize = 0;

    for (int i = 0; i < _usedEventSize; i++) {
        faster_event_t* event = _usedEvents.front();
        _usedEvents.pop_front();
        delete event;
    }
    _usedEventSize = 0;
}


void FasterEventPool::initEventPool(int numEvent, int capacityEvBuf) {

    _eventBufCapacity = capacityEvBuf;

    for (int i = 0; i < numEvent; i++) {
        faster_event_t* event = new faster_event_t();
        if (event == NULL) {
            std::cerr << "initEventPool failed: " << i << std::endl;
            break;
        }

        event->capacity = capacityEvBuf;
        event->evbuf = new char[capacityEvBuf];
        memset(event->evbuf, 0, event->capacity);
        event->size = 0;

        _freeEvents.emplace_back(event);
        _freeEventSize++;
    }
}


faster_event_t* FasterEventPool::getOneFreeEvent() {
    if (!_freeEvents.empty()) {

        faster_event_t* event = _freeEvents.front();
        _freeEvents.pop_front();
        _freeEventSize--;

        _usedEvents.emplace_back(event);
        _usedEventSize++;
        if (_addrToIter.find(event) == _addrToIter.end()) {
            _addrToIter[event] = std::prev(_usedEvents.end());
        } else {
            std::cerr << "maybe forgot to erase before." << std::endl;
            _addrToIter[event] = std::prev(_usedEvents.end());
        }
        return event;
    } else {

        faster_event_t* event = new faster_event_t();
        if (event == NULL) {
            std::cerr << "getOneFreeEvent failed." << std::endl;
            return nullptr;
        }

        event->capacity = _eventBufCapacity;
        event->evbuf = new char[_eventBufCapacity];
        memset(event->evbuf, 0, event->capacity);
        event->size = 0;



        _usedEvents.emplace_back(event);
        _usedEventSize++;

        if (_addrToIter.find(event) == _addrToIter.end()) {
            _addrToIter[event] = std::prev(_usedEvents.end());
        } else {
            std::cerr << "It should be almost impossible to happen here: " << event << std::endl;
            _addrToIter[event] = std::prev(_usedEvents.end());
        }

        return event;
    }
}


void FasterEventPool::freeOneUsedEvent(faster_event_t* event) {
    if (_addrToIter.find(event) != _addrToIter.end()) {
        
        auto it = _addrToIter[event];
        _freeEvents.splice(_freeEvents.end(), _usedEvents, it);
        _freeEventSize++;
        _usedEventSize--;
        _addrToIter.erase(event);

    } else {
        std::cout << "It should be almost impossible to happen here: freeOneUsedChunk." << std::endl;
    }
}


void FasterEventPool::printUsedAndFreeSize() {
    std::cout << "free: " << _freeEventSize << std::endl;
    std::cout << "used: " << _usedEventSize << std::endl;
}



#if 0   // TEST AND DEBUG

int main() {

    FasterEventPool* mempool = new FasterEventPool();
    mempool->initEventPool(20, 1024);

    for (int i = 0; i < 100; i++) {
        faster_event_t *event1 = mempool->getOneFreeEvent();
        faster_event_t *event2 = mempool->getOneFreeEvent();
        mempool->freeOneUsedEvent(event1);
        mempool->printUsedAndFreeSize();
    }

    delete mempool;
    return 0;
}

#endif







