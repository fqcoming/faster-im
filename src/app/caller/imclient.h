#ifndef __IMCLIENT__        
#define __IMCLIENT__

#include <string>
#include <unordered_map>
#include <iostream>
#include <functional>

#include "user.hpp"
#include "rpcchannel.h"
#include "calluserservice.h"

using namespace std;


UserServiceCaller* caller = nullptr;


// Control the main menu page program
bool isMainMenuRunning = false;


// Record the current system login user information
User g_currentUser;



// Obtain system time (chat information requires adding time information)
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}





// "help" command handler
void help()
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}


// "chat" command handler
void chat(string str)
{
    int idx = str.find(":"); // friendid:message
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    if (!caller->OneChat(friendid, message)) {
        cerr << "send chat msg error -> " << message << endl;
    }
}

// "creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx) {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    if (!caller->CreateGroup(groupname, groupdesc)) {
        cerr << "send creategroup msg error -> " << groupname << ":" << groupdesc << endl;
    }
}


// "addgroup" command handler  addgroup:groupid
void addgroup(int clientfd, string str)
{   
    int userid = g_currentUser.getId();
    int groupid = atoi(str.c_str());

    if (!caller->AddGroup(userid, groupid)) {
        cerr << "send addgroup msg error -> " << buffer << endl;
    } else {
        cout << "add group successfully." << endl;
    }
}


// "groupchat" command handler   groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}



// "loginout" command handler
void loginout(int clientfd, string)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        cerr << "send loginout msg error -> " << buffer << endl;
    } else {
        isMainMenuRunning = false;
    }   
}





// List of client commands supported by the system
unordered_map<string, string> commandMap = {
        {"help",        "Display all supported command:     [help]"                           },
        {"chat",        "Format of one-on-one chat command: [chat:friendid:message]"          },
        {"addfriend",   "Add friend command format:         [addfriend:friendid]"             },
        {"creategroup", "Create group command format:       [creategroup:groupname:groupdesc]"},
        {"addgroup",    "Add group command format:          [addgroup:groupid]"               },
        {"groupchat",   "Group chat command format:         [groupchat:groupid:message]"      },
        {"loginout",    "Loginout command Format:           [loginout]"                       }
    };



// Client command processing supported by the registration system
unordered_map<string, function<void(string)>> commandHandlerMap = {
        {"help",        help       },
        {"chat",        chat       },
        {"addfriend",   addfriend  },
        {"creategroup", creategroup},
        {"addgroup",    addgroup   },
        {"groupchat",   groupchat  },
        {"loginout",    loginout   }
    };




// Main chat page program
void mainMenu()
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;   // Storage Command

        int idx = commandbuf.find(":");
        if (-1 == idx) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // Call the event processing callback of the corresponding command, 
        // mainMenu is closed for modification, 
        // and adding new functions does not require modifying the function.
        it->second(commandbuf.substr(idx + 1, commandbuf.size() - idx)); 
    }
}




#endif