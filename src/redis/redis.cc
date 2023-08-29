
#include <iostream>

#include "redis/redis.h"

using namespace std;

Redis::Redis()
    : _publish_context(nullptr), _subcribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr) {
        redisFree(_publish_context);
    }
    if (_subcribe_context != nullptr) {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect()
{
    // Responsible for the context connection of publishing messages
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // Responsible for the context connection of subscribe subscription messages
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _subcribe_context) {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // In a separate thread, listen for events on the channel and report any messages to the business layer.
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;
    return true;
}

// Publish messages to the channel specified by Redis
bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// Subscribe to messages on the channel specified by Redis
bool Redis::subscribe(int channel)
{
    // The SUBSCRIBE command itself causes threads to block and wait for messages to occur in the channel. 
    // Here, only subscription channels are used and channel messages are not received

    // The reception of channel messages is dedicated to the observer_channel_independent threads in the message function.

    // Only responsible for sending commands and not blocking the reception of Redis server response messages, 
    // otherwise it will preempt response resources with the notifyMsg thread
    
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }

    // RedisBufferWrite can loop through sending buffers until the buffer data is sent (done is set to 1)
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    // redisGetReply

    return true;
}


// Unsubscribe messages from the channel specified by Redis
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel)) {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }

    // RedisBufferWrite can loop through sending buffers until the buffer data is sent (done is set to 1)
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done)) {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// Receive messages from subscription channels in independent threads
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        // The message received by the subscription is an array with three elements
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // Report messages that occur on the channel to the business layer
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}


void Redis::init_notify_handler(function<void(int,string)> fn) {
    this->_notify_message_handler = fn;
}