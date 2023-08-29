#ifndef __FASTERNET__
#define __FASTERNET__

#include <memory>
#include <iostream>
#include <liburing.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "faster_eventpool.h"
#include "faster_threadpool.h"
#include "faster_connpool.h"




#define ENTRIES_LENGTH				4096
#define ENTRIES_COMPLETE_LENGTH		10



class FasterTcpServer final {
public:
	FasterTcpServer() = default;
	FasterTcpServer(std::function<void(faster_event_t*)> onConnection, 
					std::function<void(faster_event_t*)> offConnection, 
					std::function<void(faster_event_t*)> onMessage, 
					std::function<void(faster_event_t*)> sendCompleted);
	~FasterTcpServer();

	// prepare before loop: 
	// start connpool + threadpool + mempool 
	bool init(std::string ip, short port, int connNum, int eventNum, int evbufMemSize, int threadNum);
	
	// loop after init
	void startLoop();

	void sendMessage(faster_event_t* event); 

	void setOnConnectionCallback(std::function<void(faster_event_t*)> onConnection);
	void setOffConnectionCallback(std::function<void(faster_event_t*)> offConnection);
	void setOnMessageCallback(std::function<void(faster_event_t*)> onMessage);
	void setSendCompletedCallback(std::function<void(faster_event_t*)> sendCompleted);


private:
	void set_accept_event(faster_event_t* ev, int flags);
	void set_write_event (faster_event_t* ev, int flags);
	void set_read_event  (faster_event_t* ev, int flags);

private:
	struct io_uring                    _ring;
	struct io_uring_params             _params;

	faster_conn_t*                     _listenConn;
	faster_event_t*                    _acceptEvent;

	std::unique_ptr<FasterConnPool>    _connpool;
	std::unique_ptr<FasterThreadPool>  _threadpool;
	std::unique_ptr<FasterEventPool>   _eventpool;

	// When opening a TCP connection, there is no need to notify the upper level application, 
	// as the upper level application only needs to save the connection information 
	// after receiving a login message and successfully logging in.
	// Therefore, there is no need for onConnection callback.

	// Close connection normally: The server receives a logout command.
	// Abnormal connection closure: The server did not receive a logout command but received a TCP packet to close the connection.

	std::function<void(faster_event_t*)> _onConnection;
	std::function<void(faster_event_t*)> _offConnection;
	std::function<void(faster_event_t*)> _onMessage;
	std::function<void(faster_event_t*)> _sendCompleted;
};



#endif