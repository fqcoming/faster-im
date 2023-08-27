#ifndef __RPCPROVIDER__
#define __RPCPROVIDER__


#include <google/protobuf/service.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

#include "faster_tcpserver.h"



class RpcProvider
{
public:

    void NotifyService(google::protobuf::Service *service);

    // Start the rpc service node and start providing rpc services.
    void Run(std::string serverIp, uint16_t serverPort, std::string zkIp, uint16_t zkPort);


private:

    void OnConnection(faster_event_t* event);

    // Established connection user's read write message event callback
    void OnMessage(faster_event_t* event);

    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);


private:

    ZkClient _zkClient;
    FasterTcpServer* _tcpServer;

    struct ServiceInfo
    {
        google::protobuf::Service* _service; // Save Service Object
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> _methodMap; // Save Service Method
    };

    // Store all information about successfully registered service objects and their service methods
    std::unordered_map<std::string, ServiceInfo> _serviceMap;
};




#endif