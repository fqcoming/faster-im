
#include <iostream>
#include "db/db.h"


// Database configuration information
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";


MySQL::MySQL() {
    _conn = mysql_init(nullptr); // Just opening up a connected resource space, 
                                 // without establishing a connection with the MySQL server.
}



MySQL::~MySQL() {
    if (_conn != nullptr)
        mysql_close(_conn);
}


bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(),    // ip
                                  user.c_str(),             // User name: root
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr) {
        // The default encoding character for C and C++ is ASCII.
        mysql_query(_conn, "set names gbk");
        std::cout << "connect mysql success!" << std::endl;
    } else {
        std::cout << "connect mysql fail!" << std::endl;
    }

    return p;
}


bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << sql << "Update failed!" << std::endl;
        return false;
    }

    return true;
}


MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << sql << "Query failed!" << std::endl;
        return nullptr;
    }
    
    return mysql_use_result(_conn);
}


MYSQL* MySQL::getConnection() {
    return _conn;
}