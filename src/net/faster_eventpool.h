#ifndef __FASTER_MEM_POOL_H__
#define __FASTER_MEM_POOL_H__

#include <list>
#include <unordered_map>
#include "faster_connpool.h"



enum {
    UNKNOWN_EV = 0,
	READ_EV    = 1, 
    WRITE_EV   = 2, 
    ACCEPT_EV  = 3, 
};


struct faster_event_t {
    faster_event_t() : seq(0), evtype(UNKNOWN_EV), evbuf(nullptr), size(0), capacity(0), conn(nullptr) {}
	~faster_event_t() { if (evbuf != nullptr) delete evbuf; };

	uint64_t seq;  // Identify whether the current event has expired. 
	               // Expiration refers to the connection corresponding to the event being released.
	int   evtype;
	char* evbuf;
	int   size;      // The current buffer stores size data.
    int   capacity;  // Maximum memory size for storing data.

	faster_conn_t* conn;
};




// Memory pool for single threading
class FasterEventPool final
{
public:
    FasterEventPool();
    ~FasterEventPool();
    void initEventPool(int numEvent, int capacityEvBuf);   // new

    faster_event_t* getOneFreeEvent();
    void freeOneUsedEvent(faster_event_t* ev);

    void printUsedAndFreeSize();

    FasterEventPool(const FasterEventPool& rhs) = delete;
    FasterEventPool& operator=(const FasterEventPool& rhs) = delete;

private:
    std::list<faster_event_t*> _freeEvents;
    int _freeEventSize;

    std::list<faster_event_t*> _usedEvents;
    int _usedEventSize;

    int _eventBufCapacity;

    // Only used for _usedMemChunk.
    std::unordered_map<faster_event_t*, std::list<faster_event_t*>::iterator> _addrToIter; 
};














#endif