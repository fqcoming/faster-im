#include "mprpccontroller.h"

FasterRpcController::MprpcController()
{
    m_failed = false;
    m_errText = "";
}

void FasterRpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool FasterRpcController::Failed() const
{
    return m_failed;
}

std::string FasterRpcController::ErrorText() const
{
    return m_errText;
}

void FasterRpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}

// 目前未实现具体的功能
void FasterRpcController::StartCancel(){}
bool FasterRpcController::IsCanceled() const {return false;}
void FasterRpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}


