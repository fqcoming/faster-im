#ifndef __RPCCHANNEL_H__
#define __RPCCHANNEL_H__

#include <unordered_map>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <semaphore.h>
#include "pb/rpcheader.pb.h"

#define BUFFER_LENGTH	1024
#define EVENTS_LENGTH	100



class FasterRpcChannel : public google::protobuf::RpcChannel {
public:
    FasterRpcChannel(std::string zkIp, uint16_t zkPort);

    // All calls to rpc methods through stub proxy objects will execute this function, 
    // which will perform data serialization and network sending for rpc method calls
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf:: Closure* done);

private:
    void startRecvThread();

    int  openConn (std::string service_name, std::string method_name);
    void closeConn(std::string service_name, std::string method_name);

    // recv thread function
    void readTaskThreadFunc();

    void procRecvMessage(std::string message);

private:
    std::string _zkIp;
    uint16_t    _zkPort;
    std::unordered_map<std::string, int> _serviceToConn;
    std::unordered_map<int, std::string> _connToService;

    int    _epfd = -1;
    sem_t  _semResponse;  // Used for communication between read and write threads
    char   _recvBuf[BUFFER_LENGTH];
    int    _recvSize;

    int    _responseSize;
    char   _responseBuf[BUFFER_LENGTH];  // Used to pass data to the main thread
                            // Only transmit request response data, not push chat data.
};


#endif