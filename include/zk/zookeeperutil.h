#ifndef __ZOOKEEPER_UTIL_H__
#define __ZOOKEEPER_UTIL_H__

#include <zookeeper/zookeeper.h>
#include <string>


class ZkClient
{
public:
    ZkClient();
    ~ZkClient();

    // zkclient starts connecting to zkserver and establishes a session.
    void Start(std::string zkIp, uint16_t zkPort);

    // Create a znode on zkserver based on the specified path, 
    // with state=0 indicating the default creation of a permanent node.
    void Create(const char *path, const char *data, int datalen, int state = 0);

    // Obtain znode data based on the path.
    std::string GetData(const char* path);

private:
    zhandle_t* _zkhandle;
};


#endif