#ifndef __CALLUSERSERVICE__
#define __CALLUSERSERVICE__


#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <string>
#include "user.pb.h"










class UserServiceCaller
{
public:
    UserServiceCaller(::google::protobuf::RpcChannel* channel) : stub(channel) {}
    bool createConn();

    bool Login(int userid, std::string pwd);
    bool Register(uint32_t id, std::string name, std::string pwd);
    bool Logout();
    
    bool OneChat();
    bool AddFriend();

    bool CreateGroup();
    bool AddGroup();
    bool GroupChat();

private:
    faster::UserServiceRpc_Stub stub;
};




#endif