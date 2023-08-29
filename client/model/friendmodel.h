#ifndef __FRIENDMODEL_H__
#define __FRIENDMODEL_H__

#include "user.h"
#include <vector>
using namespace std;


class FriendModel
{
public:
    // Add Friend Relationship
    void insert(int userid, int friendid);

    // Return to user friend list
    vector<User> query(int userid);
};

#endif