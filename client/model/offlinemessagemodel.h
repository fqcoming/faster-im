#ifndef __OFFLINEMESSAGEMODEL_H__
#define __OFFLINEMESSAGEMODEL_H__

#include <string>
#include <vector>
using namespace std;


class OfflineMsgModel
{
public:
    // Store offline messages for users
    void insert(int userid, string msg);

    // Delete offline messages for users
    void remove(int userid);

    // Query users' offline messages
    vector<string> query(int userid);
};

#endif