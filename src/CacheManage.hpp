#pragma once
#include "LRUCache.hpp"
#include "RedisClient.hpp"
#include "KeyRecommander.hpp"
#include "WebPageSearcher.hpp"
#include <string>
#include <memory>
using std::string;

// 两级缓存管理器：优先 LRU，其次 Redis，最后文件/计算
class CacheManage
{
public:
    CacheManage(size_t lruCapa, const string &redisUri)
        : _lru(lruCapa), _redis(redisUri),
          _recommander("../data/endict.dat", "../data/cndict.dat",
                       "../data/enindex.dat", "../data/cnindex.dat"),
          _searcher("../data/webpages.dat", "../data/weboffset.dat", "../data/invertindex.dat")
    {
    }

    // 查询接口：先查 LRU，再查 Redis，再查底层
    string get(const string &key)
    {
        string value;
        // 1. LRU
        if (_lru.get(key, value))
        {
            return value;
        }
        
        // 2. Redis
        if (_redis.get(key, value))
        {
            _lru.put(key, value); // 回填 LRU
            return value;
        }
        // 3. 真正查询（落盘计算）
        value = queryFromFile(key);
        _lru.put(key, value);
        _redis.set(key, value);
        return value;
    }

    ~CacheManage() {}

private:
    // 从底层查询（调用关键字推荐 / 网页搜索）
    string queryFromFile(const string &key)
    {
        auto pos = key.find(':');
        if (pos == string::npos)
            return "{}";

        int tag = stoi(key.substr(0, pos));
        string query = key.substr(pos + 1);

        if (tag == 1)
        {
            // 关键字推荐
            bool isChinese = (unsigned char)query[0] & 0x80;
            return isChinese ? _recommander.doQueryCn(query, 5)
                             : _recommander.doQueryEn(query, 5);
        }
        else if (tag == 2)
        {
            // 网页搜索
            return _searcher.doQuery(query);
        }
        return "{}";
    }

private:
    LRUCache _lru;           // 一级缓存（进程内）
    RedisClient _redis;      // 二级缓存（分布式）
    KeyRecommander _recommander;
    WebPageSearcher _searcher;
};
