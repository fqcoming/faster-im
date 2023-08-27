#ifndef __FASTERNET__
#define __FASTERNET__

#include <memory>
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <time.h>
#include <atomic>
#include <list>
#include <atomic>
#include <liburing.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "ypipe.hpp"
#include "faster_eventpool.h"
#include "faster_threadpool.h"
#include "faster_connpool.h"




#define ENTRIES_LENGTH		4096
#define MAX_CONNECTIONS		1024
#define BUFFER_LENGTH		1024
#define QUEUE_CHUNK_SIZE    100





class FasterTcpServer {
public:
	FasterTcpServer();
	~FasterTcpServer();

	// prepare before loop: 
	// start connpool + threadpool + mempool 
	bool init(std::string ip, short port);
	
	// loop after init
	void startLoop();

	void sendMessage(faster_event_t* event);

private:
	void set_accept_event(faster_event_t* ev, int flags);
	void set_write_event (faster_event_t* ev, int flags);
	void set_read_event  (faster_event_t* ev, int flags);

private:
	struct io_uring    _ring;
	faster_conn_t*     _listenConn;
	faster_event_t*    _acceptEvent;

	FasterConnPool*    _connpool;
	FasterThreadPool*  _threadpool;
	FasterEventPool*   _eventpool;

	std::function<void(faster_event_t*)> _onMessage;
};



#if 0 // TEST AND DEBUG



#endif



#endif