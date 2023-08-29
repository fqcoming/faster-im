#ifndef __USERMODEL_H__
#define __USERMODEL_H__

#include "user.h"

// Data operation class for the User table
class UserModel {
public:

    bool insert(User &user);

    // Query user information based on user ID
    User query(int id);

    // Update user status information
    bool updateState(User user);

    // Reset user's status information
    void resetState();
};

#endif