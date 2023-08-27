
#include "faster_tcpserver.h"


FasterTcpServer::FasterTcpServer() {

}



FasterTcpServer::~FasterTcpServer() {

	delete _listenConn;
	delete _acceptEvent;

}


// prepare before loop
bool FasterTcpServer::init(std::string ip, short port) {

	// init uring
	struct io_uring_params params;
	memset(&params, 0, sizeof(params));
	io_uring_queue_init_params(ENTRIES_LENGTH, &_ring, &params);

	// start listen

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (listenfd == -1) {
		std::cerr << "listenfd socket() failed." << std::endl;
		return false;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (-1 == bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
		std::cerr << "listenfd bind() failed." << std::endl;
		return false;
	}

	listen(listenfd, 100);

	_listenConn = new faster_conn_t();
	_acceptEvent = new faster_event_t();

	_listenConn->connfd = listenfd;
	_acceptEvent->evtype = ACCEPT_EV;
	_acceptEvent->conn = _listenConn;

	set_accept_event(_acceptEvent, 0);
}
	


// loop after prepare
void FasterTcpServer::startLoop() {

	for ( ; ; ) {
		struct io_uring_cqe *cqe;
		io_uring_submit(&_ring);
		int ret = io_uring_wait_cqe(&_ring, &cqe);

		struct io_uring_cqe *cqes[10];
		int cqecount = io_uring_peek_batch_cqe(&_ring, cqes, 10);

		int i = 0;
		unsigned count = 0;
	
		for (i = 0; i < cqecount; i++) {

			cqe = cqes[i];
			count++;
			faster_event_t* ev = (faster_event_t*)cqe->user_data;

			if (ev->evtype == ACCEPT_EV) {

				int connfd = cqe->res;

				faster_conn_t* newconn = _connpool->getOneFreeConn();
				newconn->connfd = connfd;

				faster_event_t* newev = _eventpool->getOneFreeEvent();
				newev->conn = newconn;
				newev->evtype = READ_EV;
				memset(newev->evbuf, 0, newev->capacity);

				set_read_event(newev, 0);
				set_accept_event(ev, 0);

			} else if (ev->evtype == READ_EV) {

				int bytes_read = cqe->res;
				if (bytes_read == 0) {

					close(ev->conn->connfd);
					_connpool->freeOneUsedConn(ev->conn);
					_eventpool->freeOneUsedEvent(ev);

				} else if (bytes_read < 0) {

					
				} else {
					_threadpool->addEvent(ev);

					faster_event_t* newev = _eventpool->getOneFreeEvent();
					newev->conn = ev->conn;
					newev->evtype = READ_EV;
					memset(newev->evbuf, 0, newev->capacity);
					set_read_event(newev, 0);
				}
			} else if (ev->evtype == WRITE_EV) {

				_eventpool->freeOneUsedEvent(ev);

			}
		}

		io_uring_cq_advance(&_ring, count);
	} // for
}


void FasterTcpServer::sendMessage(faster_event_t* event) {
	set_write_event(event, 0);
}



void FasterTcpServer::set_accept_event(faster_event_t* ev, int flags) {
	struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
	io_uring_prep_accept(sqe, ev->conn->connfd, (struct sockaddr*)&ev->conn->clientaddr, 
			(socklen_t*)&ev->conn->clientlen, flags);
	memcpy(&sqe->user_data, &ev, sizeof(faster_event_t*));
}

void FasterTcpServer::set_write_event(faster_event_t* ev, int flags) {
	struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
	io_uring_prep_send(sqe, ev->conn->connfd, ev->evbuf, BUFFER_LENGTH, flags);
	memcpy(&sqe->user_data, &ev, sizeof(faster_event_t*));
}

void FasterTcpServer::set_read_event(faster_event_t* ev, int flags) {
	struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
	io_uring_prep_recv(sqe, ev->conn->connfd, ev->evbuf, BUFFER_LENGTH, flags);
	memcpy(&sqe->user_data, &ev, sizeof(faster_event_t*));
}







#if 0  // TEST AND DEBUG

int main() {

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, yqueue_consumer_thread_condition, NULL);
	
	FasterTcpServer server;
	server.init();
	server.start();
}



#endif


































