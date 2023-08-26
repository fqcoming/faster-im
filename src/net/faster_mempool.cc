
#include <iostream>
#include "malloc.h"
#include "faster_mempool.h"


FasterMemPool::FasterMemPool() : 
    _freeMemChunkSize(0), _usedMemChunkSize(0), _sizePerChunk(0) {
}


FasterMemPool::~FasterMemPool() {
    for (int i = 0; i < _freeMemChunkSize; i++) {
        char* chunk = _freeMemChunk.front();
        _freeMemChunk.pop_front();
        free(chunk);
    }
    _freeMemChunkSize = 0;
    for (int i = 0; i < _usedMemChunkSize; i++) {
        char* chunk = _usedMemChunk.front();
        _usedMemChunk.pop_front();
        free(chunk);
    }
    _usedMemChunkSize = 0;
}


void FasterMemPool::initMemPool(int numChunk, int sizePerChunk) {
    _sizePerChunk = sizePerChunk;
    for (int i = 0; i < numChunk; i++) {
        char* chunk = (char*)malloc(_sizePerChunk * sizeof(char));
        if (chunk == NULL) {
            std::cerr << "initMemPool failed: " << i << std::endl;
            break;
        }
        _freeMemChunk.emplace_back(chunk);
        _freeMemChunkSize++;
    }
}


char* FasterMemPool::getOneFreeChunk() {
    if (!_freeMemChunk.empty()) {

        char* chunk = _freeMemChunk.front();
        _freeMemChunk.pop_front();
        _freeMemChunkSize--;

        _usedMemChunk.emplace_back(chunk);
        _usedMemChunkSize++;
        if (_addrToIter.find(chunk) == _addrToIter.end()) {
            _addrToIter[chunk] = std::prev(_usedMemChunk.end());
        } else {
            std::cerr << "maybe forgot to erase before: " << chunk << std::endl;
            _addrToIter[chunk] = std::prev(_usedMemChunk.end());
        }
        return chunk;
    } else {

        char* chunk = (char*)malloc(_sizePerChunk * sizeof(char));
        _usedMemChunk.emplace_back(chunk);
        _usedMemChunkSize++;

        if (_addrToIter.find(chunk) == _addrToIter.end()) {
            _addrToIter[chunk] = std::prev(_usedMemChunk.end());
        } else {
            std::cerr << "It should be almost impossible to happen here: " << chunk << std::endl;
            _addrToIter[chunk] = std::prev(_usedMemChunk.end());
        }
        return chunk;

    }
}


void FasterMemPool::freeOneUsedChunk(char* chunk) {
    if (_addrToIter.find(chunk) != _addrToIter.end()) {
        
        auto it = _addrToIter[chunk];
        _freeMemChunk.splice(_freeMemChunk.end(), _usedMemChunk, it);
        _freeMemChunkSize++;
        _usedMemChunkSize--;
        _addrToIter.erase(chunk);

    } else {
        std::cout << "It should be almost impossible to happen here: freeOneUsedChunk." << std::endl;
    }
}


void FasterMemPool::printUsedAndFreeSize() {
    std::cout << "free: " << _freeMemChunkSize << std::endl;
    std::cout << "used: " << _usedMemChunkSize << std::endl;
}



#if 0   // TEST AND DEBUG

int main() {

    FasterMemPool* mempool = new FasterMemPool();
    mempool->initMemPool(20, 1024);

    for (int i = 0; i < 100; i++) {
        char *chunk1 = mempool->getOneFreeChunk();
        char *chunk2 = mempool->getOneFreeChunk();
        mempool->freeOneUsedChunk(chunk1);
        mempool->printUsedAndFreeSize();
    }

    delete mempool;
    return 0;
}

#endif







