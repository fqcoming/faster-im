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

    // 目前未实现具体的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed; // RPC方法执行过程中的状态
    std::string m_errText; // RPC方法执行过程中的错误信息
};











#endif