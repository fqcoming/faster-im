#ifndef __GROUPMODEL_H__
#define __GROUPMODEL_H__

#include "group.h"
#include <string>
#include <vector>
using namespace std;


class GroupModel
{
public:

    bool createGroup(Group &group);

    void addGroup(int userid, int groupid, string role);

    // Query user group information
    vector<Group> queryGroups(int userid);

    // Query the list of group user IDs based on the specified groupID. 
    // Except for the userID itself, the main user group chats business 
    // and sends messages to other members of the group.
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif


