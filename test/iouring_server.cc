

// io_uring, tcp server

// multhread, select/poll, epoll, coroutine, iouring
// reactor

// io 

#include <liburing.h>

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

#define ENTRIES_LENGTH		4096

#define MAX_CONNECTIONS		1024
#define BUFFER_LENGTH		10480

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


int main() {

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
				
				set_read_event(&ring, connfd, buffer, BUFFER_LENGTH, 0);

				set_accept_event(&ring, listenfd, (struct sockaddr*)&clientaddr, &clilen, 0);

			} else if (ci.type == READ) {

				int bytes_read = cqe->res;
				if (bytes_read == 0) {
					close(ci.connfd);
				} else if (bytes_read < 0) {

				} else {
					//printf("buffer : %s\n", buffer);
					char *buffer = buf_table[ci.connfd];
					set_write_event(&ring, ci.connfd, buffer, bytes_read, 0);
				}
			} else if (ci.type == WRITE) {

				char *buffer = buf_table[ci.connfd];
				set_read_event(&ring, ci.connfd, buffer, BUFFER_LENGTH, 0);

			}
			

		}
		
		io_uring_cq_advance(&ring, count);
	}
	

}





