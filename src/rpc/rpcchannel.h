#ifndef __RPCCHANNEL_H__
#define __RPCCHANNEL_H__

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rpcheader.pb.h"

#define BUFFER_LENGTH	1024
#define EVENTS_LENGTH	100
#define MAX_CONNECTIONS	100




class FasterRpcChannel : public google::protobuf::RpcChannel
{
public:

    FasterRpcChannel(std::string zkIp, uint16_t zkPort);

    // All calls to rpc methods through stub proxy objects will execute this function, 
    // which will perform data serialization and network sending for rpc method calls
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf:: Closure* done);

    void startRecvThread();

private:
    int startConnWithProvider(std::string service_name, std::string method_name);
    void endConnWithProvider(std::string service_name, int clientfd);

    // recv thread function
    void readTaskHandler();
    void procRecvMessage(std::string message);

private:
    std::string _zkIp;
    uint16_t    _zkPort;
    std::unordered_map<std::string, int> _serviceToConn;

    // reactor
    int epfd = -1;
};


#endif