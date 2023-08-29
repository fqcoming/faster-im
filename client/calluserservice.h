#ifndef __CALLUSERSERVICE__
#define __CALLUSERSERVICE__


#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <string>
#include "pb/user.pb.h"





class UserServiceCaller
{
public:
    UserServiceCaller(::google::protobuf::RpcChannel* channel) : stub(channel) {}

    bool Login(uint64_t userid, std::string pwd);
    bool Register(std::string name, std::string pwd);
    bool Logout(uint64_t userid);
    
    bool OneChat(uint64_t userid, std::string username, uint64_t friendid, std::string msg);
    bool AddFriend(uint64_t userid, uint64_t friendid);

    bool CreateGroup(uint64_t userid, std::string groupname, std::string groupdesc);
    bool AddGroup(uint64_t userid, uint64_t groupid);
    bool GroupChat(uint64_t userid, uint64_t groupid, std::string msg);

private:
    faster::UserServiceRpc_Stub stub;
};




#endif