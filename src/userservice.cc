
#include <functional>

#include "redis/redis.h"
#include "model/groupmodel.h"
#include "model/friendmodel.h"
#include "model/usermodel.h"
#include "model/offlinemessagemodel.h"
#include "userservice.h"



UserService::UserService() {
}



bool UserService::Login(uint64_t userid, std::string pwd)
{
    User user = _userModel.query(userid);
    if (user.getId() == userid && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // The user has already logged in and duplicate login is not allowed
            return false;

        } else {
            // After the ID user successfully logs in, 
            // subscribe to the channel (ID) to Redis.
            _redis.subscribe(userid); 

            // Login successful, update user status information 
            // status: offline => online
            user.setState("online");
            _userModel.updateState(user);
            return true;
        }
    } else {

        // The user does not exist, 
        // or the user exists but the password is incorrect, 
        // resulting in login failure

        return false;
    }
}





bool UserService::Register(std::string name, std::string pwd, uint64_t& userid)
{
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // login was successful
        userid = user.getId();
        return true;
    }
    else
    {
        // login has failed
        return false;
    }
}


bool UserService::Logout(uint64_t userid)
{
    // User logout is equivalent to offline and unsubscribing to the channel in Redis.
    _redis.unsubscribe(userid); 

    // Update user status information
    User user(userid, "", "", "offline");
    _userModel.updateState(user);

    return true;
}


bool UserService::OneChat(uint64_t userid, std::string username, uint64_t friendid, std::string msg)
{
    std::string str = to_string(userid) + ":" + username + "->" + to_string(friendid) + ":" + msg;

    // Check if friendid is online
    User user = _userModel.query(friendid);
    if (user.getState() == "online")
    {
        std::string str = to_string(userid) + ":" + username + "->" + to_string(friendid) + ":" + msg;
        _redis.publish(friendid, str);
        return true;
    }

    // Not online, storing offline messages
    _offlineMsgModel.insert(friendid, str);
    return true;
}



bool UserService::AddFriend(uint64_t userid, uint64_t friendid)
{
    // Store friend information
    _friendModel.insert(userid, friendid);
    return true;
}

bool UserService::CreateGroup(uint64_t userid, std::string groupname, std::string groupdesc)
{
    // Store newly created group information
    Group group(-1, groupname, groupdesc);
    if (_groupModel.createGroup(group))
    {
        // Storage group creator information
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
    return true;
}



bool UserService::AddGroup(uint64_t userid, uint64_t groupid)
{
    _groupModel.addGroup(userid, groupid, "normal");
    return true;
}


bool UserService::GroupChat(uint64_t userid, uint64_t groupid, std::string msg)
{
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    std::string str = to_string(userid) + ":" + to_string(groupid) + "->" + msg;
    for (int id : useridVec)
    {
        // Check if the toid is online
        User user = _userModel.query(id);
        if (user.getState() == "online")
        {
            _redis.publish(id, str);
        }
        else
        {
            // Store offline group messages
            _offlineMsgModel.insert(id, str);
        }
    }
    return true;
}

void UserService::Login(::google::protobuf::RpcController* controller,
                const ::faster::LoginRequest* request,
                ::faster::LoginResponse* response,
                ::google::protobuf::Closure* done) 
{
    
    int id = request->userid();
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
    
    std::string name = request->name();
    std::string pwd = request->pwd();
    uint64_t userid = 0;

    bool register_result = Register(name, pwd, userid);

    response->set_userid(userid);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(register_result);

    done->Run();

}



void UserService::Logout(::google::protobuf::RpcController* controller,
                const ::faster::LogoutRequest* request,
                ::faster::LogoutResponse* response,
                ::google::protobuf::Closure* done)
{
    uint64_t userid = request->userid();
    bool result = Logout(userid);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(result);
    done->Run();
}




void UserService::OneChat(::google::protobuf::RpcController* controller,
                const ::faster::OneChatRequest* request,
                ::faster::OneChatResponse* response,
                ::google::protobuf::Closure* done)
{
    uint64_t userid = request->userid();
    std::string username = request->username();
    uint64_t friendid = request->friendid();
    std::string msg = request->msg();

    bool result = OneChat(userid, username, friendid, msg);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(result);
    done->Run();
}

void UserService::AddFriend(::google::protobuf::RpcController* controller,
                const ::faster::AddFriendRequest* request,
                ::faster::AddFriendResponse* response,
                ::google::protobuf::Closure* done)
{
    uint64_t userid = request->userid();
    uint64_t friendid = request->friendid();
    bool result = AddFriend(userid, friendid);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(result);
    done->Run();
}



void UserService::CreateGroup(::google::protobuf::RpcController* controller,
                const ::faster::CreateGroupRequest* request,
                ::faster::CreateGroupResponse* response,
                ::google::protobuf::Closure* done)
{
    uint64_t userid = request->userid();
    std::string groupname = request->groupname();
    std::string groupdesc = request->groupdesc();
    bool result = CreateGroup(userid, groupname, groupdesc);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(result);
    done->Run();
}




void UserService::AddGroup(::google::protobuf::RpcController* controller,
                const ::faster::AddGroupRequest* request,
                ::faster::AddGroupResponse* response,
                ::google::protobuf::Closure* done)
{
    uint64_t userid = request->userid();
    uint64_t groupid = request->groupid();
    bool result = AddGroup(userid, groupid);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(result);
    done->Run();
}




void UserService::GroupChat(::google::protobuf::RpcController* controller,
                const ::faster::GroupChatRequest* request,
                ::faster::GroupChatResponse* response,
                ::google::protobuf::Closure* done)
{

    uint64_t userid = request->userid();
    uint64_t groupid = request->groupid();
    std::string msg = request->msg();
    bool result = GroupChat(userid, groupid, msg);

    faster::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(result);
    done->Run();
}



void UserService::startRedis(function<void(int,string)> fn) {
    if (_redis.connect()) {
        _redis.init_notify_handler(fn);
    }
}




















