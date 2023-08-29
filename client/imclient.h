#ifndef __IMCLIENT__        
#define __IMCLIENT__

#include <string>
#include <unordered_map>
#include <iostream>
#include <functional>

#include "model/group.h"
#include "model/user.h"
#include "rpcchannel.h"
#include "calluserservice.h"

using namespace std;



extern UserServiceCaller* caller;   // stub object pointer

extern bool isMainMenuRunning;        // Control the main menu page program
extern atomic_bool g_isLoginSuccess;   // Record whether login was successful

extern User          g_currentUser;           // Record the current system login user information
extern vector<User>  g_currentUserFriendList;
extern vector<Group> g_currentUserGroupList;

extern unordered_map<string, string> commandMap;
extern unordered_map<string, function<void(string)>> commandHandlerMap;






void mainMenu();
void showCurrentUserData();
string getCurrentTime();

void help(std::string str = "");
void chat(string str);
void addfriend(string str);
void creategroup(string str);
void addgroup(string str);
void groupchat(string str);
void logout(string str);






#endif