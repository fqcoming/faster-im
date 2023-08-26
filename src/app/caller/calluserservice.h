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

    bool Login(int userid, std::string pwd);
    bool Register(std::string name, std::string pwd);
    bool Logout(int userid);
    
    bool OneChat(int friendid, string message);
    bool AddFriend();

    bool CreateGroup();
    bool AddGroup(int userid, int groupid);
    bool GroupChat(int userid, int groupid, string message);

private:
    faster::UserServiceRpc_Stub* stub;
};




#endif