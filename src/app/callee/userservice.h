#ifndef __USERSERVICE_H__
#define __USERSERVICE_H__

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include "user.pb.h"

class UserService : public faster::UserServiceRpc
{
public:
    bool Login(int id, std::string pwd);
    bool Register(uint32_t id, std::string name, std::string pwd);
    bool Logout();
    
    bool OneChat();
    bool AddFriend();

    bool CreateGroup();
    bool AddGroup();
    bool GroupChat();

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

};








































#endif