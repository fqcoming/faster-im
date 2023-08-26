#ifndef __FASTER_MEM_POOL_H__
#define __FASTER_MEM_POOL_H__

#include <list>
#include <unordered_map>


// Memory pool for single threading
class FasterMemPool final
{
public:
    FasterMemPool();
    ~FasterMemPool();
    void initMemPool(int numChunk, int sizePerChunk);   // malloc 

    char* getOneFreeChunk();
    void freeOneUsedChunk(char* chunk);

    void printUsedAndFreeSize();

    FasterMemPool(const FasterMemPool& rhs) = delete;
    FasterMemPool& operator=(const FasterMemPool& rhs) = delete;

private:
    std::list<char*> _freeMemChunk;
    int _freeMemChunkSize;

    std::list<char*> _usedMemChunk;
    int _usedMemChunkSize;

    // Only used for _usedMemChunk.
    std::unordered_map<char*, std::list<char*>::iterator> _addrToIter; 
    int _sizePerChunk;
};














#endif