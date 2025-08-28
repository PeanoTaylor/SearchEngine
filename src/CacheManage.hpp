#pragma once
#include "LFUCache.hpp"
#include "RedisClient.hpp"
#include "KeyRecommander.hpp"
#include "WebPageSearcher.hpp"
#include <string>
#include <memory>
using std::string;

struct MyThread
{
    int id;
    thread th;
    LFUCache cache;
    LFUCache patch;
    MyThread(int tid, size_t cacheCapa, size_t patchCapa)
        : id(tid), cache(cacheCapa), patch(patchCapa) {}
};

// 两级缓存管理器：优先 LRU，其次 Redis，最后文件/计算
class CacheManage
{
public:
    CacheManage(size_t lruCapa, const string &redisUri)
        : _globalCache(lruCapa), _redis(redisUri),
          _recommander("../data/endict.dat", "../data/cndict.dat",
                       "../data/enindex.dat", "../data/cnindex.dat"),
          _searcher("../data/webpages.dat", "../data/weboffset.dat", "../data/invertindex.dat")
    {
    }

    // 查询接口：先查 LRU，再查 Redis，再查底层
    string get(MyThread *th, string &key)
    {
        string value;
        // 1. 本地缓存
        if (th->cache.get(key, value))
        {
            return value;
        }

        // 2. Redis
        if (_redis.get(key, value))
        {
            th->cache.put(key, value); // 回填 LRU
            return value;
        }
        // 3. 真正查询（落盘计算）
        value = queryFromFile(key);
        th->cache.put(key, value);
        th->patch.put(key, value);
        _redis.set(key, value);
        return value;
    }

    // 添加线程上下文
    void addThread(std::shared_ptr<MyThread> th)
    {
        _threads.push_back(th);
    }

    // 定时同步：合并patch -> exportAll -> 广播全局
    void syncCaches()
    {
        for (auto &th : _threads)
        {
            auto item = th->patch.exportData();
            for (auto &[k, v] : item)
            {
                _globalCache.put(k, v);
            }
            th->patch.clear();
        }

        // 广播全局
        auto item = _globalCache.exportData();
        for (auto &th : _threads)
        {
            for (auto &[k, v] : item)
            {
                th->cache.put(k, v);
            }
        }
    }

    // 启动后台定时同步线程
    void startSyncThread(int intervalSec = 10)
    {
        std::thread([this, intervalSec]
                    {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
                syncCaches();
            } })
            .detach();
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
    LFUCache _globalCache; // 全局主缓存
    RedisClient _redis;    // 二级缓存
    KeyRecommander _recommander;
    WebPageSearcher _searcher;
    vector<std::shared_ptr<MyThread>> _threads;
};
