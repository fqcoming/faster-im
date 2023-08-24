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
#include "public.h"



#define ENTRIES_LENGTH		4096
#define MAX_CONNECTIONS		1024
#define BUFFER_LENGTH		1024
#define QUEUE_CHUNK_SIZE    100


enum {
	READ = 1, WRITE = 2, ACCEPT = 4, UNKNOWN = 0
};





struct faster_tcp_conn_t {
	struct sockaddr_in clientaddr;
	socklen_t clilen;
	int connfd;
    faster_tcp_conn_t() : clilen(sizeof(struct sockaddr_in)) {}
};

struct faster_tcp_event_t {
	int evtype;
	char evbuf[1024];
	int len;
	faster_tcp_conn_t* conn;
	faster_tcp_event_t() : evtype(UNKNOWN), len(0), conn(nullptr) {}
};


ypipe_t<faster_tcp_event_t*, QUEUE_CHUNK_SIZE> recv_event_que;
std::mutex ypipe_mutex;
std::condition_variable ypipe_cond;


void OnMessage(faster_tcp_event_t* ev) {

    



}



void *yqueue_consumer_thread_condition(void *argv) {
	while (true) {
		faster_tcp_event_t *value;
		if (recv_event_que.read(&value)) {
            OnMessage(value);
		} else {
			std::unique_lock<std::mutex> lock(ypipe_mutex);
			ypipe_cond.wait(lock);
		}
	}
	return NULL;
}


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




















#endif