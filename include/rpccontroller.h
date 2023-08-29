#ifndef __RPC_CONTROLLER_H__
#define __RPC_CONTROLLER_H__

#include <google/protobuf/service.h>
#include <string>



class FasterRpcController : public google::protobuf::RpcController
{
public:
    FasterRpcController();
    
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    // The following functions have not been implemented
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool _failed; // status during RPC method execution
    std::string _errText; // Error messages during RPC method execution
};











#endif