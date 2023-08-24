#ifndef __RPCCHANNEL_H__
#define __RPCCHANNEL_H__

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

class FasterRpcChannel : public google::protobuf::RpcChannel
{
public:
    // 所有通过stub代理对象调用rpc方法，都会执行这个函数，由这个函数统一做rpc方法调用的数据数据序列化和网络发送 
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf:: Closure* done);
private:
    int clientfd;
};


#endif