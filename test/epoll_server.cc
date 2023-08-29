

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <unistd.h>

#include <pthread.h>
#include <sys/epoll.h>
#include <string.h>


#define BUFFER_LENGTH	10480
#define EVENTS_LENGTH	128

#define MAX_CONNECTIONS	1024	


// listenfd, clientfd
struct sock_item { // conn_item

	int fd; // clientfd

	char buffer[BUFFER_LENGTH];
	int length;
	
	int event;

	void (*recv_cb)(int fd, char *buffer, int length);
	void (*send_cb)(int fd, char *buffer, int length);

	void (*accept_cb)(int fd, char *buffer, int length);

};


struct reactor {
	int epfd;
	struct sock_item items[MAX_CONNECTIONS];
};

struct reactor r;


int main() {

// block
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);  // 
	if (listenfd == -1) return -1;
// listenfd
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(9999);

	if (-1 == bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
		return -2;
	}

	listen(listenfd, 10);

	int epfd = epoll_create(1);

	struct epoll_event ev, events[EVENTS_LENGTH];
	ev.events = EPOLLIN;
	ev.data.fd = listenfd; //

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev); // 
	

	while (1) { // 7 * 24 

		int nready = epoll_wait(epfd, events, EVENTS_LENGTH, -1); // -1, ms 
		
		int i = 0;
		for (i = 0;i < nready;i ++) {
			int clientfd= events[i].data.fd;
			
			if (listenfd == clientfd) { // accept

				//while(1) {
					struct sockaddr_in client;
					socklen_t len = sizeof(client);
					int connfd = accept(listenfd, (struct sockaddr*)&client, &len);
					if (connfd == -1) break;
					
					//printf("accept: %d\n", connfd);
					ev.events = EPOLLIN;
					ev.data.fd = connfd;
					epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
				//}
			

			} else if (events[i].events & EPOLLIN) { //clientfd

				char *buffer = r.items[clientfd].buffer;

				int n = recv(clientfd, buffer, BUFFER_LENGTH, 0);
				if (n > 0) {

					r.items[clientfd].length = n;

					ev.events = EPOLLOUT;
					ev.data.fd = clientfd;

					epoll_ctl(epfd, EPOLL_CTL_MOD, clientfd, &ev);
					
				} else if (n == 0) {

					ev.events = EPOLLIN;
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);

					close(clientfd);

				}
				
			} else if (events[i].events & EPOLLOUT) {

				char *buffer = r.items[clientfd].buffer;
				
				int sent = send(clientfd, buffer, r.items[clientfd].length, 0); //
				//printf("sent: %d\n", sent);

				ev.events = EPOLLIN;
				ev.data.fd = clientfd;

				epoll_ctl(epfd, EPOLL_CTL_MOD, clientfd, &ev);
				
				
			}

		}

	}
	


}




