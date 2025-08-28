#pragma once
#include <sw/redis++/redis++.h>
#include <memory>
#include <string>
#include <hiredis/hiredis.h>
#include <string>
using std::string;
using namespace sw::redis;
using std::make_unique;
using std::unique_ptr;

class RedisClient
{
public:
    RedisClient(const string &uri = "tcp://127.0.0.1:6379")
    {
        _redis = make_unique<sw::redis::Redis>(uri);
    }

    // 查询 key 对应的值
    bool get(const string &key, string &value)
    {
        auto val = _redis->get(key);
        if (val)
        {
            value = *val;
            return true;
        }
        return false;
    }

    void set(const string &key, const string &value, int ttl = 3600)
    {
        _redis->set(key, value);
        _redis->expire(key, ttl); // 过期时间
    }

private:
    unique_ptr<sw::redis::Redis> _redis;
};
