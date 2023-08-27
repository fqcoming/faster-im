
#include <iostream>
#include "faster_connpool.h"




FasterConnPool::FasterConnPool() : _freeConnQueSize(0), _usedConnQueSize(0) {
}


FasterConnPool::~FasterConnPool() {
    for (int i = 0; i < _freeConnQueSize; i++) {
        faster_conn_t* conn = _freeConnQue.front();
        _freeConnQue.pop_front();
        free(conn);
    }
    _freeConnQueSize = 0;
    for (int i = 0; i < _usedConnQueSize; i++) {
        faster_conn_t* conn = _usedConnQue.front();
        _usedConnQue.pop_front();
        free(conn);
    }
    _usedConnQueSize = 0;
}

void FasterConnPool::initConnPool(int numConn) {
    for (int i = 0; i < numConn; i++) {
        faster_conn_t* conn = new faster_conn_t();
        _freeConnQue.emplace_back(conn);
        _freeConnQueSize++;
    }
}



faster_conn_t* FasterConnPool::getOneFreeConn() {
    if (!_freeConnQue.empty()) {
        faster_conn_t* conn = _freeConnQue.front();
        _freeConnQue.pop_front();
        _freeConnQueSize--;

        _usedConnQue.emplace_back(conn);
        _usedConnQueSize++;
        _addrToIter[conn] = std::prev(_usedConnQue.end());
        return conn;
    } else {
        faster_conn_t* conn = new faster_conn_t();
        _usedConnQue.emplace_back(conn);
        _usedConnQueSize++;
        _addrToIter[conn] = std::prev(_usedConnQue.end());
        return conn;
    }
}



void FasterConnPool::freeOneUsedConn(faster_conn_t* conn) {
    auto it = _addrToIter[conn];
    _freeConnQue.splice(_freeConnQue.end(), _usedConnQue, it);
    _freeConnQueSize++;
    _usedConnQueSize--;
    _addrToIter.erase(conn);
}




void FasterConnPool::printUsedAndFreeSize() {
    std::cout << "free: " << _freeConnQueSize << std::endl;
    std::cout << "used: " << _usedConnQueSize << std::endl;
}




#if 0   // TEST AND DEBUG

int main() {

    FasterConnPool* connpool = new FasterConnPool();
    connpool->initConnPool(10);

    for (int i = 0; i < 50; i++) {
        faster_conn_t* conn1 = connpool->getOneFreeConn();
        faster_conn_t* conn2 = connpool->getOneFreeConn();
        connpool->freeOneUsedConn(conn1);
        connpool->printUsedAndFreeSize();
    }


    delete connpool;

    return 0;
}


#endif
