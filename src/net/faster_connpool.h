#ifndef __FASTER_CONN_POOL_H__
#define __FASTER_CONN_POOL_H__


#include <netinet/in.h>
#include <sys/socket.h>


enum {
	READ    = 1, 
    WRITE   = 2, 
    ACCEPT  = 3, 
    UNKNOWN = 0
};


struct faster_conn_t {

	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	int connfd;

    faster_conn_t() : clientlen(sizeof(struct sockaddr_in)), connfd(-1) {}
};


struct faster_event_t {
    faster_event_t() : evtype(UNKNOWN), len(0), conn(nullptr) {}

	int evtype;
	char evbuf[1024];
	int len;
	faster_conn_t* conn;
};



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