

#include "faster_im.h"


// 先假定同一个连接对于发送数据，是按提交顺序执行的吗？不一定
// 提交时间之后，失败一定会收到通知吗？这里涉及到收发数据缓冲区回收时机。



class FasterTcpServer {
public:
	FasterTcpServer() = default;

	// prepare before loop
	void init(int conn_num = 1024) {
		_conn_pool.init_conn_pool(1024);
	}
	
	// loop after prepare
	void start() {
		if(!start_listen()) {
			std::cout << "listen failed." << std::endl;
			exit(-1);
		}
		struct io_uring_params params;
		memset(&params, 0, sizeof(params));

		// struct io_uring ring;
		io_uring_queue_init_params(ENTRIES_LENGTH, &_ring, &params);


		faster_tcp_event_t* accept_ev = new faster_tcp_event_t();
		accept_ev->evtype = ACCEPT;
		accept_ev->conn = &_listen_conn;
		set_accept_event(accept_ev, 0);

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
				count ++;
				faster_tcp_event_t* ev = (faster_tcp_event_t*)cqe->user_data;

				if (ev->evtype == ACCEPT) {

					int connfd = cqe->res;
					faster_tcp_conn_t* newconn = _conn_pool.get_one_free_conn();
					
					newconn->connfd = connfd;
					faster_tcp_event_t* newev = new faster_tcp_event_t();

					newev->conn = newconn;
					newev->evtype = READ;
					memset(newev->evbuf, 0, 1024);

					set_read_event(newev, 0);
					set_accept_event(ev, 0);

				} else if (ev->evtype == READ) {

					int bytes_read = cqe->res;
					if (bytes_read == 0) {
						close(ev->conn->connfd);
						_conn_pool.recyle_one_conn(ev->conn);
						delete ev;

					} else if (bytes_read < 0) {

						
					} else {

						recv_event_que.write(ev, false);
						
						if (!recv_event_que.flush()) {
							std::unique_lock<std::mutex> lock(ypipe_mutex);
							ypipe_cond.notify_one();
						}

						faster_tcp_event_t* newev = new faster_tcp_event_t();

						newev->conn = ev->conn;
						newev->evtype = READ;
						memset(newev->evbuf, 0, 1024);

						set_read_event(newev, 0);
					}
				} else if (ev->evtype == WRITE) {
					delete ev;
				}
			}
			io_uring_cq_advance(&_ring, count);
		} // for
	}

private:


	bool start_listen() {
		int listenfd = socket(AF_INET, SOCK_STREAM, 0);  
		if (listenfd == -1) return false;

		struct sockaddr_in servaddr;
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(9999);
		if (-1 == bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
				return false;
		}

		listen(listenfd, 10);
		_listen_conn.connfd = listenfd;
		return true;
	}

	void set_accept_event(faster_tcp_event_t* ev, unsigned flags) {
		struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
		io_uring_prep_accept(sqe, ev->conn->connfd, (struct sockaddr*)&ev->conn->clientaddr, (socklen_t*)&ev->conn->clilen, flags);
		memcpy(&sqe->user_data, &ev, sizeof(faster_tcp_event_t*));
	}

	void set_write_event(faster_tcp_event_t* ev, int flags) {
		struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
		io_uring_prep_send(sqe, ev->conn->connfd, ev->evbuf, BUFFER_LENGTH, flags);
		memcpy(&sqe->user_data, &ev, sizeof(faster_tcp_event_t*));
	}

	void set_read_event(faster_tcp_event_t* ev, int flags) {
		struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
		io_uring_prep_recv(sqe, ev->conn->connfd, ev->evbuf, BUFFER_LENGTH, flags);
		memcpy(&sqe->user_data, &ev, sizeof(faster_tcp_event_t*));
	}

private:
	struct io_uring        _ring;
	faster_tcp_conn_t      _listen_conn;
	faster_tcp_conn_pool_t _conn_pool;
};







int main() {

    // 首先先开启一个线程用于处理消息队列信息
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, yqueue_consumer_thread_condition, NULL);
	
	FasterTcpServer server;
	server.init();
	server.start();
}






































