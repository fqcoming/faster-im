#include "imclient.h"




int main(int argc, char *argv[]) {

    const std::string zkIp = "127.0.0.1";
    const uint16_t zkPort = 2181;
    caller = new UserServiceCaller(new FasterRpcChannel(zkIp, zkPort));
    if (caller == nullptr) {
        std::cerr << "create UserServiceCaller failed." << std::endl;
        return -1;
    }

    for (;;)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1.login"    << endl;
        cout << "2.register" << endl;
        cout << "3.quit"     << endl;
        cout << "========================" << endl;
        cout << "please choice: ";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1: // login业务
        {
            int userid = 0;
            char pwd[50] = {0};
            cout << "userid: ";
            cin >> userid;
            cin.get(); // 读掉缓冲区残留的回车
            cout << "userpassword: ";
            cin.getline(pwd, 50);

            if (caller->Login(userid, pwd)) {
                // 进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu();
            } else {
                std::cout << "login failed." << std::endl;
            }
        }
        break;
        case 2: // register业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username: ";
            cin.getline(name, 50);
            cout << "userpassword: ";
            cin.getline(pwd, 50);

            if (!caller->Register(name, pwd)) {
                std::cerr << "send reg msg error:" << request << std::endl;
            }

        }
        break;
        case 3: // quit业务
            caller->Logout();
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }

    return 0;
}








