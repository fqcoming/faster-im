#ifndef __FASTER_CONN_POOL_H__
#define __FASTER_CONN_POOL_H__

#include <list>
#include <unordered_map>
#include <netinet/in.h>
#include <sys/socket.h>




struct faster_conn_t {
	uint64_t seq;    // Used to establish consistency with events
	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	int connfd;
    faster_conn_t() : seq(0), clientlen(sizeof(struct sockaddr_in)), connfd(-1) {}
};


class FasterConnPool 
{
public:
	FasterConnPool();
	~FasterConnPool();

	void initConnPool(int numConn);
	faster_conn_t* getOneFreeConn();
	void freeOneUsedConn(faster_conn_t* conn);

	void printUsedAndFreeSize();

    FasterConnPool(const FasterConnPool& rhs) = delete;
    FasterConnPool& operator=(const FasterConnPool& rhs) = delete;

private:
	std::list<faster_conn_t*> _freeConnQue;
	int _freeConnQueSize;
	std::list<faster_conn_t*> _usedConnQue;
	int _usedConnQueSize;
	std::unordered_map<faster_conn_t*, std::list<faster_conn_t*>::iterator> _addrToIter;
};




#endif