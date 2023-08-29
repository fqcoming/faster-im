#ifndef __GROUPUSER_H__
#define __GROUPUSER_H__

#include "user.h"

// Group user, with an additional role information, 
// directly inherits from the User class and reuses user information.
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    string role;
};

#endif