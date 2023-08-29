
#include "rpcchannel.h"
#include "imclient.h"



UserServiceCaller* caller = nullptr;   // stub object pointer

bool isMainMenuRunning = false;        // Control the main menu page program
atomic_bool g_isLoginSuccess{false};   // Record whether login was successful

User          g_currentUser;           // Record the current system login user information
vector<User>  g_currentUserFriendList;
vector<Group> g_currentUserGroupList;





// List of client commands supported by the system
unordered_map<string, string> commandMap = {
        {"help",        "Display all supported command:     [help]"                           },
        {"chat",        "Format of one-on-one chat command: [chat:friendid:message]"          },
        {"addfriend",   "Add friend command format:         [addfriend:friendid]"             },
        {"creategroup", "Create group command format:       [creategroup:groupname:groupdesc]"},
        {"addgroup",    "Add group command format:          [addgroup:groupid]"               },
        {"groupchat",   "Group chat command format:         [groupchat:groupid:message]"      },
        {"logout",      "Logout command Format:             [logout]"                         }
    };



// Client command processing supported by the registration system
unordered_map<string, function<void(string)>> commandHandlerMap = {
        {"help",        help       },
        {"chat",        chat       },
        {"addfriend",   addfriend  },
        {"creategroup", creategroup},
        {"addgroup",    addgroup   },
        {"groupchat",   groupchat  },
        {"logout",      logout     },
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

        // The string before ":" is the command, followed by the request data.
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


// Display basic information of the current successfully logged in user.
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" 
         << g_currentUser.getName()     << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty()) {
        for (User &user : g_currentUserFriendList) {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty()) {
        for (Group &group : g_currentUserGroupList) {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers()) {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " "          << user.getRole()        << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}



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
void help(std::string str)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap) {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}




// "chat" command handler
void chat(string str)
{
    int idx = str.find(":");   // friendid:message
    if (-1 == idx) {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    if (!caller->OneChat(g_currentUser.getId(), g_currentUser.getName(), friendid, message)) {
        cerr << "send chat msg error -> " << message << endl;
    }
}




// "addfriend" command handler
void addfriend(string str)
{
    int friendid = atoi(str.c_str());

    if (caller->AddFriend(g_currentUser.getId(), friendid)) {

    } else {
        cerr << "send addfriend msg error." << endl;
    }
}



// "creategroup" command handler  groupname:groupdesc
void creategroup(string str)
{
    int idx = str.find(":");
    if (-1 == idx) {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    if (!caller->CreateGroup(g_currentUser.getId(), groupname, groupdesc)) {
        cerr << "send creategroup msg error -> " << groupname << ":" << groupdesc << endl;
    } else {

    }
}



// "addgroup" command handler  addgroup:groupid
void addgroup(string str)
{   
    int userid = g_currentUser.getId();
    int groupid = atoi(str.c_str());

    if (!caller->AddGroup(userid, groupid)) {
        cerr << "addgroup error -> " << str << endl;
    } else {
        cout << "add group successfully." << endl;
    }
}




// "groupchat" command handler   groupid:message
void groupchat(string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    if (caller->GroupChat(g_currentUser.getId(), groupid, message)) {
        // +++

    } else {
        cerr << "send groupchat msg error -> " << str << endl;
    }
}



// "loginout" command handler
void logout(string str)
{
    if (caller->Logout(g_currentUser.getId())) {
        isMainMenuRunning = false;
    } else {
        std::cout << "logout failed." << std::endl;
    }
    
}









