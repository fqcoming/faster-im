
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
        // Display homepage menu login, registration, and exit.
        cout << "========================" << endl;
        cout << "1.login"    << endl;
        cout << "2.register" << endl;
        cout << "3.exit"     << endl;
        cout << "========================" << endl;
        cout << "please choice: ";

        int choice = 0;
        cin >> choice;
        cin.get();        // Read out the remaining Enter key in the buffer

        switch (choice)
        {
        case 1: // login
        {
            int userid = 0;
            char pwd[50] = {0};
            cout << "userid: ";
            cin >> userid;
            cin.get();         // Read out the remaining Enter key in the buffer
            cout << "userpassword: ";
            cin.getline(pwd, 50);

            if (caller->Login(userid, pwd)) {

                // Enter the chat main menu page
                isMainMenuRunning = true;
                mainMenu();

            } else {
                std::cout << "login failed." << std::endl;
            }
        }
        break;
        case 2: // register
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username: ";
            cin.getline(name, 50);
            cout << "userpassword: ";
            cin.getline(pwd, 50);

            if (caller->Register(name, pwd)) {
                
            } else {
                std::cerr << "send reg msg error: " << name << ":" << pwd << std::endl;
            }

        }
        break;
        case 3: // exit
            // +++
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }

    return 0;
}
















