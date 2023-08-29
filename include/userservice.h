#ifndef __USERSERVICE_H__
#define __USERSERVICE_H__

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include "redis/redis.h"
#include "model/groupmodel.h"
#include "model/friendmodel.h"
#include "model/usermodel.h"
#include "model/offlinemessagemodel.h"

#include "pb/user.pb.h"

class UserService : public faster::UserServiceRpc
{
public:
    UserService();

    bool Login   (uint64_t userid, std::string pwd);
    bool Register(std::string name, std::string pwd, uint64_t& userid);
    bool Logout  (uint64_t userid);
    
    bool OneChat  (uint64_t userid, std::string username, uint64_t friendid, std::string msg);
    bool AddFriend(uint64_t userid, uint64_t friendid);

    bool CreateGroup(uint64_t userid, std::string groupname, std::string groupdesc);
    bool AddGroup   (uint64_t userid, uint64_t groupid);
    bool GroupChat  (uint64_t userid, uint64_t groupid, std::string msg);

    void Login(::google::protobuf::RpcController* controller,
                    const ::faster::LoginRequest* request,
                    ::faster::LoginResponse* response,
                    ::google::protobuf::Closure* done);

    void Register(::google::protobuf::RpcController* controller,
                    const ::faster::RegisterRequest* request,
                    ::faster::RegisterResponse* response,
                    ::google::protobuf::Closure* done);

    void Logout(::google::protobuf::RpcController* controller,
                    const ::faster::LogoutRequest* request,
                    ::faster::LogoutResponse* response,
                    ::google::protobuf::Closure* done);

    void OneChat(::google::protobuf::RpcController* controller,
                    const ::faster::OneChatRequest* request,
                    ::faster::OneChatResponse* response,
                    ::google::protobuf::Closure* done);

    void AddFriend(::google::protobuf::RpcController* controller,
                    const ::faster::AddFriendRequest* request,
                    ::faster::AddFriendResponse* response,
                    ::google::protobuf::Closure* done);

    void CreateGroup(::google::protobuf::RpcController* controller,
                    const ::faster::CreateGroupRequest* request,
                    ::faster::CreateGroupResponse* response,
                    ::google::protobuf::Closure* done);

    void AddGroup(::google::protobuf::RpcController* controller,
                    const ::faster::AddGroupRequest* request,
                    ::faster::AddGroupResponse* response,
                    ::google::protobuf::Closure* done);

    void GroupChat(::google::protobuf::RpcController* controller,
                    const ::faster::GroupChatRequest* request,
                    ::faster::GroupChatResponse* response,
                    ::google::protobuf::Closure* done);

    void startRedis(function<void(int,string)> fn);

private:

    UserModel       _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel     _friendModel;
    GroupModel      _groupModel;

    Redis _redis;
};








































#endif