#ifndef __USER_H__
#define __USER_H__

#include <string>
using namespace std;

// ORM class for the User table
class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id)          { this->id = id; }
    void setName(string name)   { this->name = name; }
    void setPwd(string pwd)     { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int    getId()    { return this->id; }
    string getName()  { return this->name; }
    string getPwd()   { return this->password; }
    string getState() { return this->state; }

protected:
    int id;
    string name;
    string password;
    string state;
};

#endif