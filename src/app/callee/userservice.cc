
#include "redis.h"
#include "groupmodel.h"
#include "friendmodel.h"
#include "usermodel.h"
#include "offlinemessagemodel.h"



bool UserService::Login(int id, std::string pwd)
{
    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id); 

            // 登录成功，更新用户状态信息 state offline=>online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 该用户不存在，用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}





bool UserService::Register(uint32_t id, std::string name, std::string pwd)
{

}



bool UserService::Logout()
{


}

bool UserService::OneChat()
{

}



bool UserService::AddFriend()
{


}

bool UserService::CreateGroup()
{

}



bool UserService::AddGroup()
{

}


bool UserService::GroupChat()
{



}

void UserService::Login(::google::protobuf::RpcController* controller,
                const ::faster::LoginRequest* request,
                ::faster::LoginResponse* response,
                ::google::protobuf::Closure* done) 
{
    
    int id = request->id();
    std::string pwd = request->pwd();

    bool login_result = Login(id, pwd);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(login_result);

    done->Run();
}

void UserService::Register(::google::protobuf::RpcController* controller,
                const ::faster::RegisterRequest* request,
                ::faster::RegisterResponse* response,
                ::google::protobuf::Closure* done)
{
    




}



void UserService::Logout(::google::protobuf::RpcController* controller,
                const ::faster::LogoutRequest* request,
                ::faster::LogoutResponse* response,
                ::google::protobuf::Closure* done)
{
    




}


void UserService::OneChat(::google::protobuf::RpcController* controller,
                const ::faster::OneChatRequest* request,
                ::faster::OneChatResponse* response,
                ::google::protobuf::Closure* done)
{

}

void UserService::AddFriend(::google::protobuf::RpcController* controller,
                const ::faster::AddFriendRequest* request,
                ::faster::AddFriendResponse* response,
                ::google::protobuf::Closure* done)
{

}



void UserService::CreateGroup(::google::protobuf::RpcController* controller,
                const ::faster::CreateGroupRequest* request,
                ::faster::CreateGroupResponse* response,
                ::google::protobuf::Closure* done)
{

}




void UserService::AddGroup(::google::protobuf::RpcController* controller,
                const ::faster::AddGroupRequest* request,
                ::faster::AddGroupResponse* response,
                ::google::protobuf::Closure* done)
{

}




void UserService::GroupChat(::google::protobuf::RpcController* controller,
                const ::faster::GroupChatRequest* request,
                ::faster::GroupChatResponse* response,
                ::google::protobuf::Closure* done)
{




}




























