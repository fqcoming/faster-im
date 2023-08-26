#ifndef __FASTERNET__
#define __FASTERNET__


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <time.h>
#include <atomic>
#include <list>
#include <memory>
#include <atomic>
#include <liburing.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "ypipe.hpp"
#include "faster_threadpool.h"
#include "faster_connpool.h"




#define ENTRIES_LENGTH		4096
#define MAX_CONNECTIONS		1024
#define BUFFER_LENGTH		1024
#define QUEUE_CHUNK_SIZE    100






struct faster_tcp_conn_pool_t {
	std::list<faster_tcp_conn_t*> free_conn;
	int free_num;
	faster_tcp_conn_pool_t() : free_num(0) {}

	void init_conn_pool(int conn_num) {
		for (int i = 0; i < conn_num; i++) {
			faster_tcp_conn_t* pconn = new faster_tcp_conn_t();
			free_conn.push_back(pconn);
			free_num++;
		}
	}
	faster_tcp_conn_t* get_one_free_conn() {
		faster_tcp_conn_t* pconn = free_conn.front();
		free_conn.pop_front();
		free_num--;
		return pconn;
	}
	void recyle_one_conn(faster_tcp_conn_t* pconn) {
		free_conn.push_back(pconn);
		free_num++;
	}
};



class FasterTcpServer {
public:
	FasterTcpServer() = default;

	// prepare before loop: 
	// connpool + lock-free queue + threadpool
	void init(int conn_num = 1024);
	
	// loop after init
	void startLoop();

private:
	bool start_listen();

	void set_accept_event(faster_tcp_event_t* ev, unsigned flags);
	void set_write_event(faster_tcp_event_t* ev, int flags);
	void set_read_event(faster_tcp_event_t* ev, int flags);

private:
	struct io_uring        _ring;
	faster_conn_t      _listen_conn;
	faster_conn_pool_t _conn_pool;

	FasterThreadPool _threadpool;
};
















#endif