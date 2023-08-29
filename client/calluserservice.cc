
#include "imclient.h"
#include "calluserservice.h"
#include "rpccontroller.h"


bool UserServiceCaller::Login(uint64_t userid, std::string pwd) {

    faster::LoginRequest requst;
    faster::LoginResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    requst.set_pwd(pwd);
    stub.Login(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {

            // +++
            g_currentUser.setId(userid);
            g_currentUser.setName("");

            showCurrentUserData();

            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }
}



bool UserServiceCaller::Register(std::string name, std::string pwd) {
    
    faster::RegisterRequest requst;
    faster::RegisterResponse response;
    FasterRpcController controller;

    requst.set_name(name);
    requst.set_pwd(pwd);
    stub.Register(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "userid: " << response.userid() << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }
}



bool UserServiceCaller::Logout(uint64_t userid) {

    faster::LogoutRequest requst;
    faster::LogoutResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    stub.Logout(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "logout successfully." << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }

}



bool UserServiceCaller::OneChat(uint64_t userid, std::string username, uint64_t friendid, std::string msg) {

    faster::OneChatRequest requst;
    faster::OneChatResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    requst.set_username(username);
    requst.set_friendid(friendid);
    requst.set_msg(msg);
    stub.OneChat(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "sent to friend msg successfully: " << requst.msg() << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }

}



bool UserServiceCaller::AddFriend(uint64_t userid, uint64_t friendid) {

    faster::AddFriendRequest requst;
    faster::AddFriendResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    requst.set_friendid(friendid);
    stub.AddFriend(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "add friend successfully: " << requst.friendid() << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }

}




bool UserServiceCaller::CreateGroup(uint64_t userid, std::string groupname, std::string groupdesc) {

    faster::CreateGroupRequest requst;
    faster::CreateGroupResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    requst.set_groupname(groupname);
    requst.set_groupdesc(groupdesc);
    stub.CreateGroup(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "create group successfully" << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }

}


bool UserServiceCaller::AddGroup(uint64_t userid, uint64_t groupid) {

    faster::AddGroupRequest requst;
    faster::AddGroupResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    requst.set_groupid(groupid);
    stub.AddGroup(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "add group successfully" << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }

}



bool UserServiceCaller::GroupChat(uint64_t userid, uint64_t groupid, std::string msg) {

    faster::GroupChatRequest requst;
    faster::GroupChatResponse response;
    FasterRpcController controller;

    requst.set_userid(userid);
    requst.set_groupid(groupid);
    requst.set_msg(msg);
    stub.GroupChat(&controller, &requst, &response, nullptr);
    
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
        return false;
    } else {
        if (response.success()) {
            std::cout << "ssend group msg successfully: " << msg << std::endl;
            return true;
        } else {
            std::cout << response.result().errmsg() << std::endl;
            return false;
        }
    }

}





















