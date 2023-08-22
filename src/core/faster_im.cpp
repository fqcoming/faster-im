
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
#include <unistd.h>
#include "ypipe.hpp"


#include <liburing.h>

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

#define ENTRIES_LENGTH		4096

#define MAX_CONNECTIONS		1024
#define BUFFER_LENGTH		1024



// typedef struct _ElemQue_s{
//     char data[BUFFER_LENGTH];
// } ElemQue_t;


#define QUEUE_CHUNK_SIZE          100


ypipe_t<char*, QUEUE_CHUNK_SIZE> yqueue;
std::mutex ypipe_mutex_;
std::condition_variable ypipe_cond_;



char buf_table[MAX_CONNECTIONS][BUFFER_LENGTH] = {0};

enum {

	READ,
	WRITE,
	ACCEPT,

};

struct conninfo {
	int connfd;
	int type;
};


void set_read_event(struct io_uring *ring, int fd, void *buf, size_t len, int flags) {

	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	io_uring_prep_recv(sqe, fd, buf, len, flags);

	struct conninfo ci = {
		.connfd = fd,
		.type = READ
	};

	memcpy(&sqe->user_data, &ci, sizeof(struct conninfo));

}



void set_write_event(struct io_uring *ring, int fd, const void *buf, size_t len, int flags) {

	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	io_uring_prep_send(sqe, fd, buf, len, flags);

	struct conninfo ci = {
		.connfd = fd,
		.type = WRITE
	};

	memcpy(&sqe->user_data, &ci, sizeof(struct conninfo));

}




void set_accept_event(struct io_uring *ring, int fd,
	struct sockaddr *cliaddr, socklen_t *clilen, unsigned flags) {

	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	io_uring_prep_accept(sqe, fd, cliaddr, clilen, flags);

	struct conninfo ci = {
		.connfd = fd,
		.type = ACCEPT
	};

	memcpy(&sqe->user_data, &ci, sizeof(struct conninfo));

}




void *yqueue_consumer_thread_condition(void *argv)
{
    // int last_value = -1;
    // PRINT_THREAD_INTO();

    while (true)
    {
        char *value;
        if (yqueue.read(&value))
        {
            
            std::cout << "读取到一个数据：" << value << std::endl;
            // if (s_consumer_thread_num == 1 && s_producer_thread_num == 1 && (last_value + 1) != value) // 只有一入一出的情况下才有对比意义
            // {
            //     printf("pid:%lu, -> value:%d, expected:%d\n", pthread_self(), value, last_value + 1);
            // }
            // lxx_atomic_add(&s_count_pop, 1);
            // last_value = value;
        }
        else
        {
            // printf("%s %lu no data, s_count_pop:%d\n", __FUNCTION__, pthread_self(), s_count_pop);
            // usleep(100);

            std::unique_lock<std::mutex> lock(ypipe_mutex_);

            std::cout << "wait" << std::endl;
            //  printf("wait\n");
            ypipe_cond_.wait(lock);
            
            std::cout << "leave wait" << std::endl;
            // sched_yield();
        }

        // if (s_count_pop >= s_queue_item_num * s_producer_thread_num)
        // {
        //     // printf("%s dequeue:%d, s_count_pop:%d, %d, %d\n", 
        //     //         __FUNCTION__, last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
        //     break;
        // }
    }

    // printf("%s dequeue: last_value:%d, s_count_pop:%d, %d, %d\n", __FUNCTION__,
    //        last_value, s_count_pop, s_queue_item_num, s_consumer_thread_num);
    
    // PRINT_THREAD_LEAVE();
    return NULL;
}









int main() {


    // 首先先开启一个线程用于处理消息队列信息

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, yqueue_consumer_thread_condition, NULL);







	int listenfd = socket(AF_INET, SOCK_STREAM, 0);  // 
    if (listenfd == -1) return -1;
// listenfd
    struct sockaddr_in servaddr, clientaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9999);

    if (-1 == bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
            return -2;
    }
	
	listen(listenfd, 10);

	struct io_uring_params params;
	memset(&params, 0, sizeof(params));

// epoll --> 
	struct io_uring ring;
	io_uring_queue_init_params(ENTRIES_LENGTH, &ring, &params);

	socklen_t clilen = sizeof(clientaddr);
	set_accept_event(&ring, listenfd, (struct sockaddr*)&clientaddr, &clilen, 0);
	
	//char buffer[1024] = {0};
//
	while (1) {

		struct io_uring_cqe *cqe;

		io_uring_submit(&ring);

		int ret = io_uring_wait_cqe(&ring, &cqe);

		struct io_uring_cqe *cqes[10];
		int cqecount = io_uring_peek_batch_cqe(&ring, cqes, 10);

		int i = 0;
		unsigned count = 0;
		
		for (i = 0;i < cqecount;i ++) {

			cqe = cqes[i];
			count ++;

			struct conninfo ci;
			memcpy(&ci, &cqe->user_data, sizeof(ci));

			if (ci.type == ACCEPT) {

				int connfd = cqe->res;
				char *buffer = buf_table[connfd];
                memset(buffer, 0, 1024);

				set_read_event(&ring, connfd, buffer, 1024, 0);

				set_accept_event(&ring, listenfd, (struct sockaddr*)&clientaddr, &clilen, 0);

			} else if (ci.type == READ) {

				int bytes_read = cqe->res;
				if (bytes_read == 0) {
					close(ci.connfd);
				} else if (bytes_read < 0) {

				} else {
                    

					char *buffer = buf_table[ci.connfd];

                    std::cout << buffer << std::endl;

                    yqueue.write(buffer, false);
                    
                    if (!yqueue.flush()) {
                        std::unique_lock<std::mutex> lock(ypipe_mutex_);
                        ypipe_cond_.notify_one();
                        std::cout << "notify" << std::endl;
                    }

                    set_read_event(&ring, ci.connfd, buffer, 1024, 0);
					// set_write_event(&ring, ci.connfd, buffer, bytes_read, 0);

				}
			} else if (ci.type == WRITE) {

				// char *buffer = buf_table[ci.connfd];

				// set_read_event(&ring, ci.connfd, buffer, 1024, 0);

			}
			

		}
		
		io_uring_cq_advance(&ring, count);
	}
	

}






































