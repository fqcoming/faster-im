
#include <arpa/inet.h>
#include "faster_tcpserver.h"


FasterTcpServer::FasterTcpServer(std::function<void(faster_event_t*)> onConnection, 
								 std::function<void(faster_event_t*)> offConnection, 
								 std::function<void(faster_event_t*)> onMessage, 
								 std::function<void(faster_event_t*)> sendCompleted) {
	setOnConnectionCallback(onConnection);
	setOffConnectionCallback(offConnection);
	setOnMessageCallback(onMessage);
	setSendCompletedCallback(sendCompleted);
}

FasterTcpServer::~FasterTcpServer() {
	if (_listenConn != nullptr) {
		delete _listenConn;
	}
	if (_acceptEvent != nullptr) {
		delete _acceptEvent;
	}
}


// prepare before loop
bool FasterTcpServer::init(std::string ip, short port, int connNum, int eventNum, int evbufMemSize, int threadNum) {

	// init uring
	memset(&_params, 0, sizeof(_params));
	io_uring_queue_init_params(ENTRIES_LENGTH, &_ring, &_params);


	// start listen

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (listenfd == -1) {
		std::cerr << "listenfd socket() failed." << std::endl;
		return false;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	// servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
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


	// init threadpool, connpool, eventpool
	_connpool.reset(new FasterConnPool);
	_connpool->initConnPool(connNum);

	_eventpool.reset(new FasterEventPool);
	_eventpool->initEventPool(eventNum, evbufMemSize);

	_threadpool.reset(new FasterThreadPool);
	_threadpool->init(threadNum, _onMessage);

	return true;
}
	


// loop after prepare
void FasterTcpServer::startLoop() {

	struct io_uring_cqe *cqes[ENTRIES_COMPLETE_LENGTH];
	for ( ; ; ) {
		struct io_uring_cqe *cqe;
		io_uring_submit(&_ring);

		// std::cout << "Waiting for completed events..." << std::endl;
		int ret = io_uring_wait_cqe(&_ring, &cqe);
		int cqecount = io_uring_peek_batch_cqe(&_ring, cqes, ENTRIES_COMPLETE_LENGTH);

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
				newev->size = 0;
				memset(newev->evbuf, 0, newev->capacity);

				// After establishing a successful handshake connection three times, 
				// handle the callback function, which generally does not need to be processed.
				_onConnection(newev);

				set_read_event(newev, 0);
				set_accept_event(ev, 0);

			} else if (ev->evtype == READ_EV) {

				int bytes_read = cqe->res;
				if (bytes_read == 0) {

					// The upper layer application needs to know when disconnecting
					_offConnection(ev);

					close(ev->conn->connfd);
					_connpool->freeOneUsedConn(ev->conn);
					_eventpool->freeOneUsedEvent(ev);

				} else if (bytes_read < 0) {

					std::cout << "bytes_read < 0" << std::endl;
					
				} else {

					ev->size = bytes_read;
					ev->evtype = WRITE_EV;
					_threadpool->addEvent(ev);

					faster_event_t* newev = _eventpool->getOneFreeEvent();
					newev->conn = ev->conn;
					newev->evtype = READ_EV;
					newev->size = 0;
					memset(newev->evbuf, 0, newev->capacity);
					set_read_event(newev, 0);
				}
			} else if (ev->evtype == WRITE_EV) {

				_sendCompleted(ev);
				_eventpool->freeOneUsedEvent(ev);
			}
		}
		io_uring_cq_advance(&_ring, count);
	} // for
}


void FasterTcpServer::sendMessage(faster_event_t* event) {
	event->evtype = WRITE_EV;
	set_write_event(event, 0);
	io_uring_submit(&_ring);
}


void FasterTcpServer::set_accept_event(faster_event_t* ev, int flags) {
	struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
	io_uring_prep_accept(sqe, ev->conn->connfd, (struct sockaddr*)&ev->conn->clientaddr, 
			(socklen_t*)&ev->conn->clientlen, flags);
	memcpy(&sqe->user_data, &ev, sizeof(faster_event_t*));
}

void FasterTcpServer::set_write_event(faster_event_t* ev, int flags) {
	struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
	io_uring_prep_send(sqe, ev->conn->connfd, ev->evbuf, ev->size, flags);
	memcpy(&sqe->user_data, &ev, sizeof(faster_event_t*));
}

void FasterTcpServer::set_read_event(faster_event_t* ev, int flags) {
	struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
	io_uring_prep_recv(sqe, ev->conn->connfd, ev->evbuf, ev->capacity, flags);
	memcpy(&sqe->user_data, &ev, sizeof(faster_event_t*));
}



void FasterTcpServer::setOnConnectionCallback(std::function<void(faster_event_t*)> onConnection) {
	_onConnection = onConnection;
}


void FasterTcpServer::setOffConnectionCallback(std::function<void(faster_event_t*)> offConnection) {
	_offConnection = offConnection;
}


void FasterTcpServer::setOnMessageCallback(std::function<void(faster_event_t*)> onMessage) {
	_onMessage = onMessage;
}


void FasterTcpServer::setSendCompletedCallback(std::function<void(faster_event_t*)> sendCompleted) {
	_sendCompleted = sendCompleted;
}





#if 0  // TEST AND DEBUG


class ApplicationServer
{
public:
	ApplicationServer() {
		_tcpServer.setOnConnectionCallback(std::bind(&ApplicationServer::onConnection, this, std::placeholders::_1));
		_tcpServer.setOffConnectionCallback(std::bind(&ApplicationServer::offConnection, this, std::placeholders::_1));
		_tcpServer.setOnMessageCallback(std::bind(&ApplicationServer::onMessage, this, std::placeholders::_1));
		_tcpServer.setSendCompletedCallback(std::bind(&ApplicationServer::sendCompleted, this, std::placeholders::_1));
	}
	~ApplicationServer() {}

	void startServer() {
		_tcpServer.init("192.168.40.137", 9999, 100, 200, 1024, 4);
		_tcpServer.startLoop();
	}

private:
	void onConnection(faster_event_t* ev) {
		std::cout << "There is a new connection coming in." << std::endl;
	}
	void offConnection(faster_event_t* ev) {
		std::cout << "A connection has been closed." << std::endl;
	}

	void onMessage(faster_event_t* ev) {
		std::cout << "recv: " << ev->evbuf << std::endl;
		_tcpServer.sendMessage(ev);
	}
	void sendCompleted(faster_event_t* ev) {
		std::cout << "Successfully sent a message for a connection." << std::endl;
	}

private:
	FasterTcpServer _tcpServer;
};


// g++ -o app faster_connpool.cc faster_eventpool.cc faster_threadpool.cc faster_tcpserver.cc -luring

int main() {

	ApplicationServer app;
	app.startServer();

	return 0;
}



#endif


































