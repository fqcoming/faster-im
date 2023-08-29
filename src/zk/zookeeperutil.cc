
#include <semaphore.h>
#include <iostream>
#include "zk/zookeeperutil.h"


// Notification from global watcher observer zkserver to zkclient.
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
	// The message type: ZOO_SESSION_EVENT is related to the session
    if (type == ZOO_SESSION_EVENT) {
		// Successful connection between zkclient and zkserver.
		if (state == ZOO_CONNECTED_STATE) {
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
		}
	} else if (type == ZOO_CHANGED_EVENT || type == ZOO_DELETED_EVENT) {
		std::cout << "node: " << path << " changed or deleted!" << std::endl;
	}
}


ZkClient::ZkClient() : _zkhandle(nullptr) {
}


ZkClient::~ZkClient() {
    if (_zkhandle != nullptr) {
        zookeeper_close(_zkhandle); // Close handle and release resources
    }
}


void ZkClient::Start(std::string zkIp, uint16_t zkPort)
{
    std::string connstr = zkIp + ":" + std::to_string(zkPort);
    
	/* zookeeper_mt: multi-thread version */

	// When a session is terminated for some reason, 
	// the temporary nodes created during this session will disappear.

	// If the zk server does not receive any messages from this session after time t = 30000 ms, 
	// the service will declare that the session has expired.

	// param1: zk host address: ip:port;
	// param2: Watchpoint functions for handling events;
	// param3: Session timeout: 30000ms;
	// param4: Specify the parameter nullptr to start a new session;
	// param5: Returns the context object used by the zkhandle handle;
	// param6: This parameter is not currently in use, so setting it to 0 is sufficient.

    _zkhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == _zkhandle) {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(_zkhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}



void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;

	// First, determine whether the znode represented by path exists. 
	// If it does, it will not be created again.

	// param3: If set to 0, it has no effect. 
	// If set to non 0, the zookeeper server will set a monitor to notify clients 
	// when a node changes.

	flag = zoo_exists(_zkhandle, path, 0, nullptr);

	if (ZNONODE == flag) // the path does not exist
	{
		flag = zoo_create(_zkhandle, path, data, datalen,
				&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);

		if (flag == ZOK) {
			std::cout << "znode create success... path: " << path << std::endl;
		} else {
			std::cout << "flag: " << flag << std::endl;
			std::cout << "znode create error... path: " << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}


std::string ZkClient::GetData(const char* path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(_zkhandle, path, 0, buffer, &bufferlen, nullptr);

	if (flag != ZOK) {
		std::cout << "get znode error... path: " << path << std::endl;
		return "";
	} else {
		return buffer;
	}
}




#define DEBUG	0
#if DEBUG

// g++ -o zookeeperutil zookeeperutil.cc -lzookeeper_mt

int main() {

	ZkClient zkclient;
	zkclient.Start("127.0.0.1", 2181);
	std::string str = zkclient.GetData("/zookeeper");
	std::cout << str << std::endl;

	zkclient.Create("/share", "127.0.0.1:9999", 14, 0);
	zkclient.Create("/work", "127.0.0.1:9999", 14, ZOO_EPHEMERAL);

	str = zkclient.GetData("/share");
	std::cout << str << std::endl;

	getchar();
	return 0;
}



#endif

#if defined DEBUG
#undef DEBUG
#endif