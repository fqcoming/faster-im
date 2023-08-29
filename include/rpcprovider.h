#ifndef __RPCPROVIDER__
#define __RPCPROVIDER__


#include <google/protobuf/service.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

#include "userservice.h"
#include "faster_tcpserver.h"
#include "zk/zookeeperutil.h"


class RpcProvider
{
public:
    RpcProvider();
    ~RpcProvider();

    void NotifyService(google::protobuf::Service *service);

    // Start the rpc service node and start providing rpc services.
    void StartService(std::string serverIp, uint16_t serverPort, std::string zkIp, uint16_t zkPort);


private:

    void OnConnectionCallback(faster_event_t* event);
    void OffConnectionCallback(faster_event_t* event);

    // Established connection user's read write message event callback
    void OnMessageCallback(faster_event_t* event);
    void SendCompletedCallback(faster_event_t* event);

    // Closure callback operation, used to serialize rpc response and network sending
    void SendRpcResponse(faster_event_t* event, google::protobuf::Message* response);

    // Get subscribed messages from the Redis message queue
    void handleRedisSubscribeMessage(uint64_t userid, std::string msg);

private:

    ZkClient _zkClient;
    std::unique_ptr<FasterTcpServer> _tcpServer;

    struct ServiceInfo
    {
        google::protobuf::Service* _service; // Save Service Object
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> _methodMap; // Save Service Method
    };

    // Store all information about successfully registered service objects and their service methods
    std::unordered_map<std::string, ServiceInfo> _serviceMap;

    std::unordered_map<uint64_t, faster_conn_t*> _userConnMap;
    std::mutex _mapMutex;  
    
};




#endif