#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;


// When Redis is used as a publish subscribe message queue for cluster server communication, 
// it will encounter two difficult bugs. Please refer to my blog for a detailed description:
// https://blog.csdn.net/QIANGWEIYUAN/article/details/97895611


class Redis
{
public:
    Redis();
    ~Redis();

    // Connect to Redis server
    bool connect();

    // Publish messages to the channel specified by Redis
    bool publish(int channel, string message);

    // Subscribe to messages on the channel specified by Redis
    bool subscribe(int channel);

    // Unsubscribe messages from the channel specified by Redis
    bool unsubscribe(int channel);

    // Receive messages from subscription channels in independent threads
    void observer_channel_message();

    // Initialize the callback object for reporting channel messages to the business layer
    void init_notify_handler(function<void(int, string)> fn);

private:
    // Hiredis synchronizes the context object, responsible for publishing messages
    redisContext *_publish_context;

    // Hiredis synchronizes the context object and is responsible for subscribing messages
    redisContext *_subcribe_context;

    // Callback operation, receive subscription messages and report to the service layer
    function<void(int, string)> _notify_message_handler;
};

#endif
