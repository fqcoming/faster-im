
#include "rpccontroller.h"


FasterRpcController::FasterRpcController() : _failed(false), _errText("") {
}


void FasterRpcController::Reset() {
    _failed = false;
    _errText = "";
}


bool FasterRpcController::Failed() const {
    return _failed;
}

std::string FasterRpcController::ErrorText() const {
    return _errText;
}

void FasterRpcController::SetFailed(const std::string& reason) {
    _failed = true;
    _errText = reason;
}


void FasterRpcController::StartCancel() {}
bool FasterRpcController::IsCanceled() const { return false; }
void FasterRpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}


